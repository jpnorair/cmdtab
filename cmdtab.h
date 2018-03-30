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
  * @file       cmdtab.h
  * @author     JP Norair
  * @version    R100
  * @date       1 Apr 2018
  * @brief      Command Table Interface
  * @ingroup    CMDTAB
  *
  ******************************************************************************
  */

#ifndef _cmdtab_h
#define _cmdtab_h

#include <stdint.h>
#include <stdlib.h>


///@todo possible to make these compile-time options
#define CMDTAB_NAME_MAX		32 
#define CMDTAB_ALLOC_CHUNK	32


typedef struct {
	char*   name; 
	void*   action; 
	void*   extcmd;
} cmdtab_item_t;

typedef struct {
	cmdtab_item_t**	cmd;
	size_t			size;
	size_t			alloc;
} cmdtab_t;




int cmdtab_init(cmdtab_t* table);

int cmdtab_add(cmdtab_t* table, const char* name, void* action, void* extcmd);

void cmdtab_free(cmdtab_t* table);

const cmdtab_item_t* cmdtab_search(const cmdtab_t* table, const char *cmdname);

const cmdtab_item_t* cmdtab_subsearch(const cmdtab_t* table, char *namepart);




#endif
