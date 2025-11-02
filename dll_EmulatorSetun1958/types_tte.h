/*
****************************************************************************
*
* Filename: 'types_tte.h'  Заголовочный файл. Типы данных.*
* Copyright (c) 2018 All rights reserved.
* Software License Agreement.
*
*****************************************************************************
*/
/*
*  Authors:   Васильев В.И.
*  Creation Date:   25.04.2018 14:28
*  Last Date:       29.04.2018 11:40
*  Version:         0.0.2
*  Description:  Проект Setun-1958
*/

#ifndef _TYPES_TTE_H_
#define _TYPES_TTE_H_

#define TRUE 1
#define FALSE 0

#define P_NULL ( (void *) 0)
#define NULL_P ( (void *) 0)

typedef const unsigned char code_area;
typedef const unsigned char prog_char;

typedef signed char S8;
typedef unsigned char U8;
typedef signed int S16;
typedef unsigned int U16;
typedef signed long S32;
typedef unsigned long U32;
typedef signed long long S64;
typedef unsigned long long U64;

#endif /* _TYPES_TTE_H_ */
