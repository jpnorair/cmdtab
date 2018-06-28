/*  Copyright 2018, JP Norair
  *
  * Licensed under the OpenTag License, Version 1.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.indigresso.com/wiki/doku.php?id=opentag:license_1_0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  */
  
/**
  * @file       cmdtab.c
  * @author     JP Norair
  * @version    R100
  * @date       1 Apr 2013
  * @brief      Command Table Implementation
  * @ingroup    CMDTAB
  *
  ******************************************************************************
  */

#include "cmdtab.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


// comapres two strings by alphabet,
// returns 0 - if equal, -1 - first one bigger, 1 - 2nd one bigger.
static int local_strcmp(const char *s1, const char *s2);

// comapres first x characters of two strings by alphabet,
// returns 0 - if equal, -1 - first one bigger, 1 - 2nd one bigger.
static int local_strcmpc(const char *s1, const char *s2, int x);


cmdtab_item_t* cmdtab_search_insert(cmdtab_t* table, const char *cmdname, bool do_insert);





int cmdtab_init(cmdtab_t* table) {

	if (table == NULL) {
		return -1;
	}

	table->cmd		= NULL;
	table->size 	= 0;
	table->alloc	= 0;
	table->cmd 		= malloc(CMDTAB_ALLOC_CHUNK*sizeof(cmdtab_item_t*));
	
	if (table->cmd == NULL) {
		return -1;
	}
	
	table->alloc = CMDTAB_ALLOC_CHUNK;
	return 0;
}




void cmdtab_free(cmdtab_t* table) {
	int i;
	
	if (table != NULL) {
		if (table->cmd != NULL) {
		
			i = (int)table->size;
			while (--i >= 0) {
				cmdtab_item_t* cmditem;
		
				cmditem = table->cmd[i];
				if (cmditem != NULL) {
					if (cmditem->name != NULL) {
						free(cmditem->name);
					}
					free(cmditem);
				}
			}
		
			free(table->cmd);
		}
		
		table->cmd		= NULL;
		table->size 	= 0;
		table->alloc	= 0;
	}
}



int cmdtab_add(cmdtab_t* table, const char* name, void* action, void* extcmd) {
	cmdtab_item_t* newcmd;
	
	if ((table == NULL) || (name == NULL)) {
		return -1;
	}
	
	///1. Make sure there is room in the table
	if (table->alloc <= table->size) {
		cmdtab_item_t** newtable_cmd;
		size_t          newtable_alloc;
		
		newtable_alloc  = table->alloc + CMDTAB_ALLOC_CHUNK;
		newtable_cmd    = realloc(table->cmd, newtable_alloc * sizeof(cmdtab_item_t*));
		if (newtable_cmd == NULL) {
			return -2;
		}
		
		table->cmd 		= newtable_cmd;
		table->alloc	= newtable_alloc;
	}
	
	///2. Insert a new cmd item, then fill the leftover items.
	///   cmd_search_insert allocates and loads the name field.
	newcmd = cmdtab_search_insert(table, name, true);
	
	if (newcmd != NULL) {
		newcmd->action = action;
		newcmd->extcmd = extcmd;
		return 0;
	}
	
	return -2;
}


int cmdtab_list(cmdtab_t* table, char* dst, size_t dstmax) {
    int i;
    int chars_out = 0;
    
    for (i=0; i<table->size; i++) {
        int name_size;
        name_size   = strlen(table->cmd[i]->name);
        chars_out  += name_size;
        
        if (chars_out < dstmax) {
            dst     = stpncpy(dst, table->cmd[i]->name, name_size);
            dst[0]  = '\n';
            dst++;
        }
        else {
            break;
        }
    }
    
    return chars_out;
}



const cmdtab_item_t* cmdtab_search(const cmdtab_t* table, const char *cmdname) {
///@note cmdtab_search_insert takes a non-const cmdtab_t*, but if the "do_insert"
///      argument is false, it will not modify the table in any way.
	return cmdtab_search_insert((cmdtab_t*)table, cmdname, false);
}



const cmdtab_item_t* cmdtab_subsearch(const cmdtab_t* table, char *namepart) {

    if (*namepart != 0) {
        int len = (int)strlen(namepart);

        // try to find single match
        int l   = 0;
        int r   = (int)table->size - 1;
        int lr  = -1;
        int rr  = -1;
        int cci;
        int csc;
        
        while(r >= l) {
            cci = (l + r) >> 1;
            csc = local_strcmpc((char*)table->cmd[cci]->name, namepart, len);
            
            switch (csc) {
                case -1: r = cci - 1; break;
                case  1: l = cci + 1; break;
                    
                // check for matches left and right
                default:
                    if (cci > 0) {
                        lr = local_strcmpc((char*)table->cmd[cci-1]->name, namepart, len);
                    }
                    if (cci < (table->size-1)) {
                        rr = local_strcmpc((char*)table->cmd[cci+1]->name, namepart, len);
                    }
                    return (lr & rr) ? table->cmd[cci] : NULL;
            }
        }
    }
        
    return NULL;
}





cmdtab_item_t* cmdtab_search_insert(cmdtab_t* table, const char *cmdname, bool do_insert) {
/// An important condition of using this function with insert is that table->alloc must
/// be greater than table->size.
///
/// Verify that cmdname is not a zero-length string, then search for it in the
/// list of available commands.
///
    cmdtab_item_t** head;
    cmdtab_item_t* output = NULL;
	int cci = 0;
    int csc = -1;

	if ((table == NULL) || (cmdname == NULL)) {
		return NULL;
	}

    if (*cmdname != 0) {
        int l   = 0;
        int r   = (int)table->size - 1;
        head    = table->cmd;
    
        while (r >= l) {
            cci = (l + r) >> 1;
            csc = local_strcmp((char*)(head[cci]->name), cmdname);
            
            switch (csc) {
                case -1: r = cci - 1; 
                         break;
                
                case  1: l = cci + 1; 
                         break;
                
                default: output = head[cci];
                         goto cmdtab_search_insert_DO;
            }
        }
        
        /// Adding a new cmdtab_item_t
        /// If there is a matching name (output != NULL), the new one will replace old.
        /// It will adjust the table and add the new item, otherwise.
        cmdtab_search_insert_DO:
        if (do_insert) {
        	size_t newcmd_len;
        	char* newcmd_name;
        
        	// Allocate the new command name
        	newcmd_len = strlen(cmdname);
			if (newcmd_len > CMDTAB_NAME_MAX) {
				newcmd_len = CMDTAB_NAME_MAX;
			}
        	newcmd_name = malloc(newcmd_len);
        	if (newcmd_name == NULL) {
        		return NULL;
        	}
			strncpy(newcmd_name, cmdname, newcmd_len);

        	// If the command name is unique, then it will not be found in the search
        	// above, and output == NULL.
        	if (output == NULL) {
        		cci += (csc > 0);
        		for (int i=table->size; i>cci; i--) {
        			head[i] = head[i-1];
        		}
        		
        		output = malloc(sizeof(cmdtab_item_t));
        		if (output == NULL) {
        			free(newcmd_name);
        			return NULL;
        		}
        		head[cci] = output;
        		table->size++;
        	}
        	
        	// Command was found in search above, so we must free the old command name
        	// before replacing it.
        	else if (output->name != NULL) {
        		free(output->name);
        	}
        	
        	// Add the new command item at the output pointer
			output->name    = newcmd_name;
			output->action  = NULL;
			output->extcmd  = NULL;
        }
    }
    
	return output;
}



static int local_strcmp(const char *s1, const char *s2) {
	for (; (*s1 == *s2) && (*s1 != 0); s1++, s2++);	
	return (*s1 < *s2) - (*s1 > *s2);
}


static int local_strcmpc(const char *s1, const char *s2, int x) {
    for (int i = 0; (*s1 == *s2) && (*s1 != 0) && (i < x - 1); s1++, s2++, i++) ;
    return (*s1 < *s2) - (*s1 > *s2);
}



#ifdef __TEST__
int main(void) {
	cmdtab_t             table;
    const cmdtab_item_t* cmditem;
	int rc;
 
    // Some extcmds to use with the test
    int extcmd0[2] = {0, 1};
    int extcmd1[2] = {2, 3};
    int extcmd2[2] = {4, 5};
    int extcmd3[2] = {6, 7};
    int extcmd4[2] = {8, 9};
    int extcmd5[2] = {10, 11};
    int extcmd6[2] = {12, 13};
    
	
	rc = cmdtab_init(&table);
	printf("cmdtab_init() returns %d\n", rc);

    printf("Test1: adding 7 commands with different extcmds, unique names\n");
    cmdtab_add(&table, "abc", NULL, extcmd0);
    cmdtab_add(&table, "oepd", NULL, extcmd1);
    cmdtab_add(&table, "m.pyd", NULL, extcmd2);
    cmdtab_add(&table, "bepxk", NULL, extcmd3);
    cmdtab_add(&table, "rthuee", NULL, extcmd4);
    cmdtab_add(&table, "srchke", NULL, extcmd5);
    cmdtab_add(&table, "ouiddf", NULL, extcmd6);
    
    cmditem = cmdtab_search(&table, "abc");
    if (cmditem == NULL) {
        printf("search for \"abc\" failed\n");
    }
    else {
        printf("\"abc\" was found\n");
        printf("    cmd->extcmd[0] = %d\n", ((int*)cmditem->extcmd)[0]);
        printf("    cmd->extcmd[1] = %d\n", ((int*)cmditem->extcmd)[1]);
    }
    
    
    /// Free the table.  There is a lot of dynamic memory.
    cmdtab_free(&table);
    
    return 0;
}
#endif



