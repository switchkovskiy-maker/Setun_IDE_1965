/*
*****************************************************************************
*
* Filename: 'objects.c'
* Description: Объекты
*
* Copyright (c) 2018  All rights reserved.
* Software License Agreement.
*
******************************************************************************
*/
/*
*  Authors: Васильев В.И.
*  Creation Date:   25.04.2018 15:11
*  Last Date:       26.04.2018 09:09
*  Version:         0.0.2
*  Description:  Проект Setun-1958
*/

/* ------------------------------------------------------------------------------
*  Системные заголовки
*  ------------------------------------------------------------------------------
*/
/* ------------------------------------------------------------------------------
*  Локальные заголовки
*  ------------------------------------------------------------------------------
*/
#include "types_tte.h"
#include "setun_tte.h"

/* ------------------------------------------------------------------------------
*  Определения
*  ------------------------------------------------------------------------------
*/
/* ------------------------------------------------------------------------------
*  Переменные
*  ------------------------------------------------------------------------------
*/
/* ------------------------------------------------------------------------------
*  Функции
*  ------------------------------------------------------------------------------
*/
/* ****************************
*  
*  ****************************
*/
S8 not( S8 v) {
   if(v > 0) {
    return 0;
   }
   else {
    return 1;
   }
}

void bte_00_fn( S8 a, bte_00_st_t * bte_00)
{
  bte_00->y5 = a;
}

void bte_g1_fn(bte_g1_st_t * bte_g1)
{
  bte_g1->y5 = 1;
}

void bte_1_fn( S8 a, S8 b, bte_1_st_t * bte_1)
{
  bte_1->y5 = a * not(b);
}

void bte_2_fn( S8 a, S8 b, bte_2_st_t * bte_2)
{
  bte_2->y5 = a * b;
}

void bte_3_fn( S8 a, S8 b, bte_3_st_t * bte_3)
{
  bte_3->y5 = a | b;
}

void tte_0_fn( S8 a, S8 b, tte_0_st_t * tte_0)
{
  tte_0->y5 = b * not(a);
  tte_0->y7 = a * not(b);
}

void tte_00_fn( S8 a, S8 b, S8 c, S8 d, tte_00_st_t * tte_00)
{
  tte_00->y5 = d * not(c);
  tte_00->y6 = b * not(a);
  tte_00->y7 = c * not(d);
  tte_00->y8 = a * not(b);  
}

void tte_01_fn( S8 a, S8 b, tte_01_st_t * tte_01)
{
  tte_01->y5 = b * not(a);
  tte_01->y6 = a * not(b);
  tte_01->y7 = a * not(b);
  tte_01->y8 = a * not(b);
}

void tte_02_fn( S8 a, S8 b, tte_02_st_t * tte_02)
{
  tte_02->y5 = b * not(a);
  tte_02->y7 = a * not(b);
}

void tte_03_fn( S8 a, S8 b, S8 c, S8 d, tte_03_st_t * tte_03)
{
  tte_03->y5 = (b+d) * not(a) + b*d;
  tte_03->y7 = (a+c) * not(b) + a*c;
} 

void tte_1_fn( tte_1_st_t * tte_1)
{
  tte_1->y7 = 1;
} 

void tte_11_fn( tte_11_st_t * tte_11)
{
  tte_11->y5 = 1;
  tte_11->y7 = 1;
} 

void tte_2_v1_fn( S8 a, S8 b, S8 c, S8 d, tte_2_v1_st_t * tte_2_v1)
{
  tte_2_v1->y8 = a * not(b) * not(c) * not(d);
}

void tte_2_v2_fn( S8 a, S8 b, S8 c, S8 d, tte_2_v2_st_t * tte_2_v2)
{
  tte_2_v2->y8 = (a+b) * not(c)* not(d) + a*b * ( not(c)+not(d) );
}

void tte_2_v3_fn( S8 a, S8 b, S8 c, S8 d, tte_2_v3_st_t * tte_2_v3)
{
  tte_2_v3->y8 = (a+b+c) * not(d) + a*b + a*c + b*c;
}

void tte_22_fn( S8 a, S8 b, S8 c, S8 d, tte_22_st_t * tte_22)
{
  tte_22->y6 = b * not(d);
  tte_22->y8 = a * not(c);
}

void tte_23_fn( S8 a, S8 b, S8 c, tte_23_st_t * tte_23)
{
  tte_23->y6 = b * not(c);
  tte_23->y8 = a * not(c);
}

void tte_24_fn( S8 a, S8 b, S8 c, tte_24_st_t * tte_24)
{
  tte_24->y6 = b * not(a) * not(c);
  tte_24->y8 = a * not(c) * not(c);
}

void tte_27_fn( S8 a, S8 b, S8 c, tte_27_st_t * tte_27)
{
  tte_27->y6 = b * not(c);
  tte_27->y8 = a * not(c);
}

void tte_28_fn( S8 a, S8 b, S8 c, tte_28_st_t * tte_28)
{
  tte_28->y6 = b * not(a) * not(c);
  tte_28->y8 = a * not(b) * not(c);
}

void tte_4_v1_fn( S8 a, S8 b, S8 c, S8 d, tte_4_v1_st_t * tte_4_v1)
{
  tte_4_v1->y8 = a*b+ a*c + a*d + b*c + b*d + c*d;
}

void tte_4_v2_fn( S8 a, S8 b, S8 c, S8 d, tte_4_v2_st_t * tte_4_v2)
{
  tte_4_v2->y8 = (a*b+ a*c + b*c) * not(b) + a*b*c;
}

void tte_4_v3_fn( S8 a, S8 b, S8 c, S8 d, tte_4_v3_st_t * tte_4_v3)
{
  tte_4_v3->y8 = a*b*not(c)*not(b);
}

void tte_43_fn( S8 a, S8 b, S8 c, tte_43_st_t * tte_43)
{
  tte_43->y6 = b*c;
  tte_43->y8 = a*c;
}

void tte_44_fn( S8 a, S8 b, S8 c, S8 d, tte_44_st_t * tte_44)
{
  tte_44->y6 = c*d;
  tte_44->y8 = a*b;
}

void tte_47_fn( S8 a, S8 b, S8 c, S8 d, tte_47_st_t * tte_47)
{
  tte_47->y6 = not(a)*b;
  tte_47->y8 = a*b;
}

void tte_71_fn( S8 a, S8 b, S8 c, S8 d, tte_71_st_t * tte_71)
{
  tte_71->y8 = a*b;
}     

/* ****************************
*  
*  ****************************
*/
void pg418_fig6_a(S8 a, S8 b, S8 *y1, S8 *y2 ) {
    bte_1_st_t bte_1;
    bte_3_st_t bte_3;
    bte_1_fn(a,b,(bte_1_st_t *)&bte_1);
    bte_3_fn(a,b,(bte_3_st_t *)&bte_3);
    *y1 = bte_1.y5;
    *y2 = bte_3.y5;
}

void pg418_fig6_b(S8 a, S8 b, S8 c, S8 d, S8 *y1, S8 *y2, S8 *y3, S8 *y4 ) {
    bte_1_st_t bte_1;
    bte_3_st_t bte_3[3];
    
    bte_1_fn(a,b|c|d,(bte_1_st_t *)&bte_1);
    bte_3_fn(a,b,(bte_3_st_t *)&bte_3[0]);
    bte_3_fn(a,c,(bte_3_st_t *)&bte_3[1]);
    bte_3_fn(a,d,(bte_3_st_t *)&bte_3[2]);
    
    *y1 = bte_1.y5;
    *y2 = bte_3[0].y5;
    *y3 = bte_3[1].y5;
    *y4 = bte_3[2].y5;    
}

void pg418_fig7(S8 a, S8 b, S8 *y1, S8 *y2, S8 *y3, S8 *y4 ) {
    
    bte_00_st_t bte_00[2];
    bte_g1_st_t bte_g1;    
    bte_1_st_t bte_1[3];
    bte_2_st_t bte_2;
    
    bte_00_fn(a,(bte_00_st_t *)&bte_00[0]);
    bte_00_fn(b,(bte_00_st_t *)&bte_00[1]);

    bte_1_fn(bte_00[0].y5,bte_00[1].y5,(bte_1_st_t *)&bte_1[0]);
    bte_1_fn(bte_00[1].y5,bte_00[0].y5,(bte_1_st_t *)&bte_1[1]);

    bte_2_fn(bte_00[0].y5,bte_00[1].y5,(bte_2_st_t *)&bte_2);
    
    bte_g1_fn((bte_g1_st_t *)&bte_g1);

    bte_1_fn(bte_g1.y5,bte_00[0].y5|bte_00[1].y5,(bte_1_st_t *)&bte_1[2]);
    
    *y1 = bte_1[0].y5;
    *y2 = bte_1[1].y5;
    *y3 = bte_2.y5;
    *y4 = bte_1[2].y5;    
}

/*
* 
*/
void trit_conter(void) {
/* TEST 2 ----------------------------- */
    {
        S8 v;
        S8 a,b,c,d;
        S8 y5,y6,y7,y8;
        S8 r;

        bte_1_st_t bte_1;
        bte_2_st_t bte_2;
        bte_3_st_t bte_3;

        tte_0_st_t tte_0;
        tte_00_st_t tte_00;
        tte_01_st_t tte_01;
        tte_02_st_t tte_02;
        tte_03_st_t tte_03;
        tte_1_st_t tte_1;
        tte_11_st_t tte_11;
        tte_2_v1_st_t tte_2_v1;
        tte_2_v2_st_t tte_2_v2;
        tte_2_v3_st_t tte_2_v3;
        tte_22_st_t tte_22;
        tte_23_st_t tte_23;
        tte_24_st_t tte_24;
        tte_27_st_t tte_27;
        tte_28_st_t tte_28;
        tte_4_v1_st_t tte_4_v1;
        tte_4_v2_st_t tte_4_v2;
        tte_4_v3_st_t tte_4_v3;
        tte_43_st_t tte_43;
        tte_44_st_t tte_44;
        tte_47_st_t tte_47;
        tte_71_st_t tte_71;

        //t2.1 Ok'
        v = 0;  //printf("v=%i\n",v);
        r = not(v);  //printf("r=%i\n",r);
        r = not(r);  //printf("r=%i\n",r);
        //t2.2 tte_0_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("tte_0() -> ");
                //printf("in(a=%i b=%i):",a,b);
                tte_0_fn( a, b, (tte_0_st_t*) &tte_0);
                //printf(" out(y5=%i y7=%i)\n",tte_0.y5,tte_0.y7);
            }
        }
        //t2.3 tte_00_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_00() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_00_fn( a, b, c, d, (tte_00_st_t*) &tte_00);
                        //printf(" out(y5=%i y6=%i y7=%i y8=%i)\n",tte_00.y5,tte_00.y6,tte_00.y7,tte_00.y8);
                    }
                }
            }
        }
        //t2.4 tte_01_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("tte_01() -> ");
                //printf("in(a=%i b=%i):",a,b);
                tte_01_fn( a, b, (tte_01_st_t*) &tte_01);
                //printf(" out(y5=%i y6=%i y7=%i y8=%i)\n",tte_01.y5,tte_01.y6,tte_01.y7,tte_01.y8);
            }
        }
        //t2.5 tte_02_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("tte_02() -> ");
                //printf("in(a=%i b=%i):",a,b);
                tte_01_fn( a, b, (tte_01_st_t*) &tte_01);
                //printf(" out(y5=%i y7=%i)\n",tte_02.y5,tte_02.y7);
            }
        }
        //t2.6 tte_03_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_03() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_03_fn( a, b, c, d, (tte_03_st_t*) &tte_03);
                        //printf(" out(y5=%i y7=%i)\n",tte_03.y5,tte_03.y7);
                    }
                }
            }
        }
        //t2.7 tte_1_fn()
        //printf("tte_1()  -> ");
        tte_1_fn( (tte_1_st_t*) &tte_1);
        //printf(" out(y7=%i)\n",tte_1.y7);
        //t2.8 tte_11_fn()
        //printf("tte_11() -> ");
        tte_11_fn( (tte_11_st_t*) &tte_11);
        //printf(" out(y5=%i y7=%i)\n",tte_11.y5,tte_11.y7);
        //t2.9 tte_2_v1_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_2_v1() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_2_v1_fn( a, b, c, d, (tte_2_v1_st_t*) &tte_2_v1);
                        //printf(" out(y8=%i)\n",tte_2_v1.y8);
                    }
                }
            }
        }
        //t2.10 tte_2_v2_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_2_v2() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_2_v2_fn( a, b, c, d, (tte_2_v2_st_t*) &tte_2_v2);
                        //printf(" out(y8=%i)\n",tte_2_v2.y8);
                    }
                }
            }
        }
        //t2.11 tte_2_v3_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_2_v3() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_2_v3_fn( a, b, c, d, (tte_2_v3_st_t*) &tte_2_v3);
                        //printf(" out(y8=%i)\n",tte_2_v3.y8);
                    }
                }
            }
        }
        //t2.12 tte_22_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_22() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_22_fn( a, b, c, d, (tte_22_st_t*) &tte_22);
                        //printf(" out(y6=%i y8=%i)\n",tte_22.y6,tte_22.y8);
                    }
                }
            }
        }
        //t2.13 tte_23_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_23() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_23_fn( a, b, c, (tte_23_st_t*) &tte_23);
                    //printf(" out(y6=%i y8=%i)\n",tte_23.y6,tte_23.y8);
                }
            }
        }
        //t2.13 tte_23_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_23() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_23_fn( a, b, c, (tte_23_st_t*) &tte_23);
                    //printf(" out(y6=%i y8=%i)\n",tte_23.y6,tte_23.y8);
                }
            }
        }
        //t2.14 tte_24_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_24() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_24_fn( a, b, c, (tte_24_st_t*) &tte_24);
                    //printf(" out(y6=%i y8=%i)\n",tte_24.y6,tte_24.y8);
                }
            }
        }
        //t2.14 tte_27_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_27() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_27_fn( a, b, c, (tte_27_st_t*) &tte_27);
                    //printf(" out(y6=%i y8=%i)\n",tte_27.y6,tte_27.y8);
                }
            }
        }
        //t2.15 tte_28_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_28() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_28_fn( a, b, c, (tte_28_st_t*) &tte_28);
                    //printf(" out(y6=%i y8=%i)\n",tte_28.y6,tte_28.y8);
                }
            }
        }
        //t2.16 tte_4_v1_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_4_v1() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_4_v1_fn( a, b, c, d, (tte_4_v1_st_t*) &tte_4_v1);
                        //printf(" out(y8=%i)\n",tte_4_v1.y8);
                    }
                }
            }
        }
        //t2.17 tte_4_v2_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_4_v2() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_4_v2_fn( a, b, c, d, (tte_4_v2_st_t*) &tte_4_v2);
                        //printf(" out(y8=%i)\n",tte_4_v2.y8);
                    }
                }
            }
        }
        //t2.18 tte_4_v3_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_4_v3() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_4_v3_fn( a, b, c, d, (tte_4_v3_st_t*) &tte_4_v3);
                        //printf(" out(y8=%i)\n",tte_4_v3.y8);
                    }
                }
            }
        }
        //t2.19 tte_43_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    //printf("tte_43() -> ");
                    //printf("in(a=%i b=%i c=%i):",a,b,c);
                    tte_43_fn( a, b, c, (tte_43_st_t*) &tte_43);
                    //printf(" out(y6=%i y8=%i)\n",tte_43.y6,tte_43.y8);
                }
            }
        }
        //t2.20 tte_44_fn()
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                        //printf("tte_44() -> ");
                        //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                        tte_44_fn( a, b, c, d, (tte_44_st_t*) &tte_44);
                        //printf(" out(y6=%i y8=%i)\n",tte_44.y6,tte_44.y8);
                    }
                }
            }
        }
        //t2.21 tte_47_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("tte_47() -> ");
                //printf("in(a=%i b=%i):",a,b);
                tte_47_fn( a, b, c, d, (tte_47_st_t*) &tte_47);
                //printf(" out(y6=%i y8=%i)\n",tte_47.y6,tte_47.y8);
            }
        }
        //t2.22 tte_71_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("tte_71() -> ");
                //printf("in(a=%i b=%i):",a,b);
                tte_71_fn( a, b, c, d, (tte_71_st_t*) &tte_71);
                //printf(" out(y8=%i)\n",tte_71.y8);
            }
        }
        //t2.23 bte_1_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("bte_1() -> ");
                //printf("in(a=%i b=%i):",a,b);
                bte_1_fn( a, b, (bte_1_st_t*) &bte_1);
                //printf(" out(y5=%i)\n",bte_1.y5);
            }
        }
        //t2.24 bte_2_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("bte_2() -> ");
                //printf("in(a=%i b=%i):",a,b);
                bte_2_fn( a, b, (bte_2_st_t*) &bte_2);
                //printf(" out(y5=%i)\n",bte_2.y5);
            }
        }
        //t2.25 bte_3_fn() Ok'
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("bte_3() -> ");
                //printf("in(a=%i b=%i):",a,b);
                bte_3_fn( a, b, (bte_3_st_t*) &bte_3);
                //printf(" out(y5=%i)\n",bte_3.y5);
            }
        }
    } /* End TEST 2 */

    /* TEST 3 ----------------------------- */
    {
    // 
    // in(a,b) out(y1,y2)
    // y1=a*not(b)
    // y2=a*b
    S8 a,b;
    S8 y1,y2;
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("pg418_fig6_a() -> ");
                //printf("in(a=%i b=%i):",a,b);
                pg418_fig6_a(a, b, &y1, &y2);
                //printf(" out(y1=%i y2=%i)\n",y1,y2);
            }
        }
    } /* End TEST 3 */

	/* TEST 4 ----------------------------- */
    { 
    //
    S8 a,b,c,d;
    S8 y1,y2,y3,y4;
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                for(c=0;c<2;c++) {
                    for(d=0;d<2;d++) {
                    //printf("pg418_fig6_b() -> ");
                    //printf("in(a=%i b=%i c=%i d=%i):",a,b,c,d);
                    pg418_fig6_b(a,b,c,d,&y1,&y2,&y3,&y4);
                    //printf(" out(y1=%i y2=%i y3=%i y4=%i)\n",y1,y2,y3,y4);
                    } 
                } 
            } 
        }
    } /* End TEST 4 */
        
    /* TEST 5 ----------------------------- */
    { 
    //
    S8 a,b;
    S8 y1,y2,y3,y4;
        for(a=0;a<2;a++) {
            for(b=0;b<2;b++) {
                //printf("pg418_fig7() -> ");
                //printf("in(a=%i b=%i):",a,b);
                pg418_fig7(a,b,&y1,&y2,&y3,&y4);  
                //printf(" out(y1=%i y2=%i y3=%i y4=%i)\n",y1,y2,y3,y4);
            } 
        }
    } /* End TEST 5 */    
}

/* EOF  "setun_tte.c" */
