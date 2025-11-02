#ifndef _SETUN_TTE_H_
#define _SETUN_TTE_H_

#include "types_tte.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------
*  Основые ферритовые логические элементы "Сетунь-1958"
*
*  1. Брусенцов Н.П., Маслов С.П., Розин В.П., Тишулина А.М.,
*     Малая цифровая вычислительная машина "Сетунь". // [Текст] .-
*     Москва .- изд. МГУ .- 1962 г. - 140 стр. - с чертеж.
*  ---------------------------------------------------------------
*/

// Переменные логического элемента с ферритами
typedef struct bte_698409_26_st
{
    U8 p1; // Вход тактирования phase 1
    U8 p2; // Вход тактирования phase 2

    // Ферритовое кольцо 1
    U8 b1; // Контакт w1.1 - флаг фронта входа с1.1
    U8 b2; // Контакт w1.2 - флаг фронта входа с1.2
    U8 b3; // Контакт w1.3 - флаг фронта входа с1.3
    U8 b4; // Контакт w1.4 - флаг фронта входа с1.4
    U8 b5; // ferrite core 1 статус
    U8 b6; // Контакт w1.5 - статус выхода с1.5
    U8 b7; // Контакт w1.6 - статус выходf c1.6

    // Ферритовое кольцо 2
    U8 b8; // Контакт w2.1 - флаг фронта входа с2.1
    U8 b9; // Контакт w2.2 - флаг фронта входа с2.2
    U8 b10;// Контакт w2.3 - флаг фронта входа с2.3
    U8 b11;// Контакт w2.4 - флаг фронта входа с2.4
    U8 b12; // ferrite core 2 статус
    U8 b13; // Контакт w2.5 - статус выхода с2.5
    U8 b14; // Контакт w2.6 - статус выходf c2.6

    // Ферритовое кольцо 3
    U8 b15; // Контакт w3.1 - флаг фронта входа с3.1
    U8 b16; // Контакт w3.2 - флаг фронта входа с3.2
    U8 b17; // Контакт w3.3 - флаг фронта входа с3.3
    U8 b18; // Контакт w3.4 - флаг фронта входа с3.4
    U8 b19; // Контакт w3.5 - флаг фронта входа с3.5
    U8 b20; // Контакт w3.6 - флаг фронта входа с3.6
    U8 b21; // ferrite core 3 статус
    U8 b22; // Контакт w3.7 - статус выхода с3.7
    U8 b23; // Контакт w3.8 - статус выходf c3.8
} bte_698409_26_st_t;

// Упрощенные структуры для совместимости с C++
typedef struct { S8 a1; S8 y5; S8 fc1; S8 fc2; S8 fc3; } bte_00_st_t;
typedef struct { S8 a1; S8 y5; S8 fc1; S8 fc2; S8 fc3; } bte_g1_st_t;
typedef struct { S8 a1; S8 b1; S8 y5; S8 fc1; S8 fc2; S8 fc3; } bte_1_st_t;
typedef struct { S8 a1; S8 b1; S8 y5; S8 fc1; S8 fc2; S8 fc3; } bte_2_st_t;
typedef struct { S8 a1; S8 b1; S8 y5; S8 fc1; S8 fc2; S8 fc3; } bte_3_st_t;
typedef struct { S8 a2; S8 b4; S8 y5; S8 y7; S8 fc1; S8 fc2; } tte_0_st_t;
typedef struct { S8 a1; S8 b3; S8 c2; S8 d4; S8 y5; S8 y6; S8 y7; S8 y8; S8 fc1; S8 fc2; } tte_00_st_t;
typedef struct { S8 a1; S8 b3; S8 y5; S8 y6; S8 y7; S8 y8; S8 fc1; S8 fc2; } tte_01_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 y5; S8 y7; S8 fc1; S8 fc2; } tte_02_st_t;
typedef struct { S8 a1; S8 b3; S8 c2; S8 c4; S8 c7; S8 d2; S8 d5; S8 y5; S8 y7; S8 fc1; S8 fc2; } tte_03_st_t;
typedef struct { S8 y7; S8 fc1; S8 fc2; } tte_1_st_t;
typedef struct { S8 y5; S8 y7; S8 fc1; S8 fc2; } tte_11_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d6; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_2_v1_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d6; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_2_v2_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d6; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_2_v3_st_t;
typedef struct { S8 a1; S8 a7; S8 b3; S8 b4; S8 c5; S8 d0; S8 d2; S8 y6; S8 y8; S8 fc1; S8 fc2; } tte_22_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 c5; S8 c7; S8 y6; S8 y8; S8 fc1; S8 fc2; } tte_23_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 c5; S8 c7; S8 y8; S8 y6; S8 fc1; S8 fc2; } tte_24_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 c5; S8 c7; S8 y8; S8 y6; S8 fc1; S8 fc2; } tte_27_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 c5; S8 c7; S8 y8; S8 y6; S8 fc1; S8 fc2; } tte_28_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d5; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_4_v1_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d5; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_4_v2_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 c4; S8 c5; S8 d6; S8 d7; S8 y8; S8 fc1; S8 fc2; } tte_4_v3_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 c5; S8 c7; S8 y6; S8 y8; S8 fc1; S8 fc2; } tte_43_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b9; S8 c3; S8 c7; S8 d4; S8 d5; S8 y6; S8 y8; S8 fc1; S8 fc2; } tte_44_st_t;
typedef struct { S8 a1; S8 a2; S8 b3; S8 b4; S8 y6; S8 y8; S8 fc1; S8 fc2; } tte_47_st_t;
typedef struct { S8 a0; S8 a1; S8 b2; S8 b3; S8 y8; S8 fc1; S8 fc2; } tte_71_st_t;

/* Прототипы функций TTE-цифровые элементы */
//S8 not(S8 v);
void bte_00_fn(S8 a, bte_00_st_t* bte_00);
void bte_g1_fn(bte_g1_st_t* bte_g1);
void bte_1_fn(S8 a, S8 b, bte_1_st_t* bte_1);
void bte_2_fn(S8 a, S8 b, bte_2_st_t* bte_2);
void bte_3_fn(S8 a, S8 b, bte_3_st_t* bte_3);
void tte_0_fn(S8 a, S8 b, tte_0_st_t* tte_0);
void tte_00_fn(S8 a, S8 b, S8 c, S8 d, tte_00_st_t* tte_00);
void tte_01_fn(S8 a, S8 b, tte_01_st_t* tte_01);
void tte_02_fn(S8 a, S8 b, tte_02_st_t* tte_02);
void tte_03_fn(S8 a, S8 b, S8 c, S8 d, tte_03_st_t* tte_03);
void tte_1_fn(tte_1_st_t* tte_1);
void tte_11_fn(tte_11_st_t* tte_11);
void tte_2_v1_fn(S8 a, S8 b, S8 c, S8 d, tte_2_v1_st_t* tte_2_v1);
void tte_2_v2_fn(S8 a, S8 b, S8 c, S8 d, tte_2_v2_st_t* tte_2_v2);
void tte_2_v3_fn(S8 a, S8 b, S8 c, S8 d, tte_2_v3_st_t* tte_2_v3);
void tte_22_fn(S8 a, S8 b, S8 c, S8 d, tte_22_st_t* tte_22);
void tte_23_fn(S8 a, S8 b, S8 c, tte_23_st_t* tte_23);
void tte_24_fn(S8 a, S8 b, S8 c, tte_24_st_t* tte_24);
void tte_27_fn(S8 a, S8 b, S8 c, tte_27_st_t* tte_27);
void tte_28_fn(S8 a, S8 b, S8 c, tte_28_st_t* tte_28);
void tte_4_v1_fn(S8 a, S8 b, S8 c, S8 d, tte_4_v1_st_t* tte_4_v1);
void tte_4_v2_fn(S8 a, S8 b, S8 c, S8 d, tte_4_v2_st_t* tte_4_v2);
void tte_4_v3_fn(S8 a, S8 b, S8 c, S8 d, tte_4_v3_st_t* tte_4_v3);
void tte_43_fn(S8 a, S8 b, S8 c, tte_43_st_t* tte_43);
void tte_44_fn(S8 a, S8 b, S8 c, S8 d, tte_44_st_t* tte_44);
void tte_47_fn(S8 a, S8 b, S8 c, S8 d, tte_47_st_t* tte_47);
void tte_71_fn(S8 a, S8 b, S8 c, S8 d, tte_71_st_t* tte_71);

void pg418_fig6_a(S8 a, S8 b, S8* y1, S8* y2);
void pg418_fig6_b(S8 a, S8 b, S8 c, S8 d, S8* y1, S8* y2, S8* y3, S8* y4);
void pg418_fig7(S8 a, S8 b, S8* y1, S8* y2, S8* y3, S8* y4);
void trit_conter(void);

#ifdef __cplusplus
}
#endif

#endif    /*  _SETUN_TTE_H_ */
