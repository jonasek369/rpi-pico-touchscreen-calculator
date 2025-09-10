/**
 * Picokalkulačka
 * (c) Jirka Chráska 2025; <jirka@lixis.cz>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

 #include "pico/stdlib.h"
 #include <stdbool.h>
 #include <stdio.h>
 #include <string.h>
 #include "ili9341.h"
 // #include "ili9341_framebuffer.h"
 #include "ili9341_draw.h"
 #include "xpt2046.h"
 #include "ugui.h"
  
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

 
 #include "tokenizer.h"
 #include "ast.h"

 // časovač obsluhy touchscreenu
 bool timer_ts_poll_callback(struct repeating_timer *t);
 
 void pixel_set(UG_S16 x, UG_S16 y, UG_COLOR rgb);
 UG_RESULT _HW_DrawLine(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR rgb);
 UG_RESULT _HW_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR rgb);
 
 // callback funkce obsluhy událostí na grafických objektech
 void window_1_callback( UG_MESSAGE* msg );
 void button_0_click(void);
 void button_1_click(void);
 void button_2_click(void);
 void button_3_click(void);
 void button_4_click(void);
 void button_5_click(void);
 void button_6_click(void);
 void button_7_click(void);
 void button_8_click(void);
 void button_9_click(void);
 void button_a_click(void);
 void button_b_click(void);
 void button_c_click(void);
 void button_d_click(void);
 void button_e_click(void);
 void button_f_click(void);
 void button_plus_click(void);
 void button_minus_click(void);
 void button_krat_click(void);
 void button_deleno_click(void);
 void button_back_click(void);
 void button_kalk_click(void);
 void button_clear_click(void);

 void button_open_parenth_click(void);
 void button_close_parenth_click(void);
 
 static UG_GUI gui;
 // definice grafických objektů
 static UG_WINDOW window_1;
 static UG_BUTTON button_0;
 static UG_BUTTON button_1;
 static UG_BUTTON button_2;
 static UG_BUTTON button_3;
 static UG_BUTTON button_4;
 static UG_BUTTON button_5;
 static UG_BUTTON button_6;
 static UG_BUTTON button_7;
 static UG_BUTTON button_8;
 static UG_BUTTON button_9;
 static UG_BUTTON button_a;
 static UG_BUTTON button_b;
 static UG_BUTTON button_c;
 static UG_BUTTON button_d;
 static UG_BUTTON button_e;
 static UG_BUTTON button_f;
 static UG_BUTTON button_plus;
 static UG_BUTTON button_minus;
 static UG_BUTTON button_krat;
 static UG_BUTTON button_deleno;
 static UG_BUTTON button_back;
 static UG_BUTTON button_kalk;
 static UG_BUTTON button_clear;
 static UG_TEXTBOX textbox_1;

 // TODO: Custom
 #define BTN_ID_23 23
 #define BTN_ID_24 24
 static UG_BUTTON button_open_parenth;
 static UG_BUTTON button_close_parenth;
 
 // pole pro vstup "displej kalkulačky"
 #define VYRAZ_SIZE 256
 int  i = 0;
 char vyraz[VYRAZ_SIZE+1] = {'\0'};
 char* stb_vyraz = NULL;

 
  
 #define MAX_OBJECTS 40
 UG_OBJECT obj_buff_wnd_1[MAX_OBJECTS];
 
 
 // oknverze barev z ARGB8888 do formátu RGB565
 uint16_t ili9341_color_565RGB(uint8_t R, uint8_t G, uint8_t B) 
 {
     uint16_t c;
     c = (((uint16_t)R)>>3)<<11 | (((uint16_t)G)>>2) << 5 | ((uint16_t)B)>>3;
     return c;
 }
 
 
 // kreslící funkce
 void pixel_set(UG_S16 x, UG_S16 y, UG_COLOR rgb)
 {
     uint16_t R = (rgb >> 16) & 0x0000FF;
     uint16_t G = (rgb >> 8) & 0x0000FF;
     uint16_t B = rgb & 0x0000FF;
     UG_COLOR RGB16 = RGBConv(B,G,R);
     draw_pixel(x,y,RGB16);
 }
 
 UG_RESULT _HW_DrawLine(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR rgb)
 {
     uint16_t R = (rgb >> 16) & 0x0000FF;
     uint16_t G = (rgb >> 8) & 0x0000FF;
     uint16_t B = rgb & 0x0000FF;
     UG_COLOR RGB16 = RGBConv(B,G,R);
     if (x1 == x2) {
         draw_vertical_line(x1,y1,y2-y1,RGB16);
     } else if (y1 == y2) {
         draw_horizontal_line(x1,y1,x2-x1,RGB16);
     } else {
         //drawLine(x1,y1,x2,y2,RGB16);
     }
     return UG_RESULT_OK;
 }
 
 UG_RESULT _HW_FillFrame(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2, UG_COLOR rgb)
 {
     uint16_t R = (rgb >> 16) & 0x0000FF;
     uint16_t G = (rgb >> 8) & 0x0000FF;
     uint16_t B = rgb & 0x0000FF;
     UG_COLOR RGB16 = RGBConv(B,G,R);
     fill_rectangle_alt(x1,x2,y1,y2,RGB16);
     return UG_RESULT_OK;
 }
 
 
 
 
 // obsluha událostí hlavního okna
 void window_1_callback( UG_MESSAGE* msg )
 {
    if ( msg->type == MSG_TYPE_OBJECT )
    {
       if ( msg->id == OBJ_TYPE_BUTTON )
       {
          switch( msg->sub_id )
          {
             case BTN_ID_0: // stisknuta 0
             {
                button_0_click();
                break;
             }
             case BTN_ID_1: // stisknuta 1
             {
                button_1_click();
                break;
             }
             case BTN_ID_2: // stisknuta 2
             {
                button_2_click();
                break;
             }
             case BTN_ID_3: // stisknuta 3
             {
                button_3_click();
                break;
             }
             case BTN_ID_4: // stisknuta 4
             {
                button_4_click();
                break;
             }
             case BTN_ID_5: // stisknuta 5
             {
                button_5_click();
                break;
             }
             case BTN_ID_6: // stisknuta 6
             {
                button_6_click();
                break;
             }
             case BTN_ID_7: // stisknuta 7
             {
                button_7_click();
                break;
             }
             case BTN_ID_8: // stisknuta 8
             {
                button_8_click();
                break;
             }
             case BTN_ID_9: // stisknuta 9
             {
                button_9_click();
                break;
             }
             case BTN_ID_10: // stisknuta a
             {
                button_a_click();
                break;
             }
             case BTN_ID_11: // stisknuta b
             {
                button_b_click();
                break;
             }
             case BTN_ID_12: // stisknuta c
             {
                button_c_click();
                break;
             }
             case BTN_ID_13: // stisknuta d
             {
                button_d_click();
                break;
             }
             case BTN_ID_14: // stisknuta e
             {
                button_e_click();
                break;
             }
             case BTN_ID_15: // stisknuta f
             {
                button_f_click();
                break;
             }
             case BTN_ID_16: // stisknuto +
             {
                button_plus_click();
                break;
             }
             case BTN_ID_17: // stisknuto -
             {
                button_minus_click();
                break;
             }
             case BTN_ID_18: // stisknuto *
             {
                button_krat_click();
                break;
             }
             case BTN_ID_19: // stisknuta /
             {
                button_deleno_click();
                break;
             }
             case BTN_ID_20: // stisknuto DEL
             {
                button_back_click();
                break;
             }
             case BTN_ID_21: // stisknuto =
             {
                button_kalk_click();
                break;
             }
             case BTN_ID_22: // stisknuto CLR
             {
                button_clear_click();
                break;
             }
             case BTN_ID_23: // stisknuto (
             {
                button_open_parenth_click();
                break;
             }
             case BTN_ID_24: // stisknuto )
             {
                button_close_parenth_click();
                break;
             }
             default : break;
          }
       }
    }
 }
 
 // obsluha stisknutí číslice 0
 void button_0_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '0';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 1
 void button_1_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '1';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 2
 void button_2_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '2';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 3
 void button_3_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '3';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 4
 void button_4_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '4';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 5
 void button_5_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '5';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 6
 void button_6_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '6';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 7
 void button_7_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '7';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 8
 void button_8_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '8';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí číslice 9
 void button_9_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '9';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // zatím nepoužito
 void button_a_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'a';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 void button_b_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'b';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 void button_c_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'c';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 void button_d_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'd';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 void button_e_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'e';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 void button_f_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = 'f';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 
 // obsluha stisknutí operátoru +
 void button_plus_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '+';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí operátoru -
 void button_minus_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '-';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí operátoru *
 void button_krat_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '*';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
 // obsluha stisknutí operátoru /
 void button_deleno_click(void)
 {
     if( i < VYRAZ_SIZE - 1 ) {
         vyraz[i] = '/';
         vyraz[i+1] = '\0';
         i++;
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 
// obsluha stisknutí tlačitka (
void button_open_parenth_click(void)
{
    if( i < VYRAZ_SIZE - 1 ) {
        vyraz[i] = '(';
        vyraz[i+1] = '\0';
        i++;
        UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
    }
}

// obsluha stisknutí tlačitka )
void button_close_parenth_click(void)
{
    if( i < VYRAZ_SIZE - 1 ) {
        vyraz[i] = ')';
        vyraz[i+1] = '\0';
        i++;
        UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
    }
}
 

 // obsluha stisknutí backspace
 void button_back_click(void)
 {
     if( i > 0 ) {
         vyraz[--i] = '\0';
         UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
     }
 }
 


 // obsluha stisknutí operátoru =
 // spuštění výpočtu
 void button_kalk_click(void)
 {
     printf("Starting tokenization!\n");

     TokenizerOutput to = tokenize(vyraz);
     ExprNode* ast = generate_ast(to.tokens);
     if(!ast){
        vyraz[0] = '\0';
        i = 0;
        UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
        return;
     }
     print_ast(ast, 0);
     float result = compute_ast(ast);

     arrfree(to.tokens);
     free(to.tokensValues);
     free_ast(ast);
 
     sprintf(vyraz, "%g", result);
     i = strlen(vyraz);
     UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
 }
 
 
 // obsluha stisknutí tlačítka clear all
 void button_clear_click(void)
 {
    vyraz[0] = '\0';
    i = 0;
    UG_TextboxSetText(&window_1, TXB_ID_22, vyraz );
 }
 
 
 // obsluha dotykového panelu
 bool timer_ts_poll_callback(struct repeating_timer *t)
 {
     if (gpio_get(TS_IRQ_PIN) == 0) {
         uint16_t x = ts_get_x();
         uint16_t y = ts_get_y();	
         if ((x > 0 && x < 239) && (y > 0 && y < 319)) {
             UG_TouchUpdate(x, y, TOUCH_STATE_PRESSED);
         }
     } else {
         UG_TouchUpdate(-1, -1, TOUCH_STATE_RELEASED);
     }
     UG_Update();
     return true;
 }
 
 
 int main() {
     stdio_init_all();
     sleep_ms(2000);
    ili9341_init();
     ts_spi_setup();
 
     printf("start\n");

     strcpy(vyraz,"Zacnete...");
 
 
     fill_screen(0x0000);
     struct repeating_timer timer;
     add_repeating_timer_ms(30, timer_ts_poll_callback, NULL, &timer);
 
     UG_Init(&gui, pixel_set, 240, 320);
 
     UG_DriverRegister( DRIVER_DRAW_LINE, (void*)_HW_DrawLine );
     UG_DriverRegister( DRIVER_FILL_FRAME, (void*)_HW_FillFrame );
     UG_DriverEnable( DRIVER_DRAW_LINE );
     UG_DriverEnable( DRIVER_FILL_FRAME );
 
     UG_FillScreen(C_LIME);
 
     // definice hlavního okna
     UG_WindowCreate( &window_1, obj_buff_wnd_1, MAX_OBJECTS, window_1_callback );
     UG_WindowSetTitleText( &window_1, "Kalkulator" );
     UG_WindowSetTitleTextFont( &window_1, &FONT_12X20 );
 
     // definice černého displeje
     UG_TextboxCreate( &window_1, &textbox_1, TXB_ID_22, 0, 0, 235, 47 );
     UG_TextboxSetFont( &window_1, TXB_ID_22, &FONT_12X20);
     UG_TextboxSetText( &window_1, TXB_ID_22, vyraz );
     UG_TextboxSetForeColor(&window_1, TXB_ID_22, C_GOLD );
     UG_TextboxSetBackColor(&window_1, TXB_ID_22, C_BLACK );
     UG_TextboxSetAlignment( &window_1, TXB_ID_22, ALIGN_H_LEFT );
 
     // definice tlačítek
     UG_ButtonCreate( &window_1, &button_7, BTN_ID_7,      0, 48, 48, 96 ); // tlačítko 7
     UG_ButtonSetFont( &window_1, BTN_ID_7, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_7, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_7, "7" );
 
     UG_ButtonCreate( &window_1, &button_8, BTN_ID_8,      48, 48, 96, 96 ); // tlačítko 8
     UG_ButtonSetFont( &window_1, BTN_ID_8, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_8, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_8, "8" );
 
     UG_ButtonCreate( &window_1, &button_9, BTN_ID_9,      96, 48, 144, 96 ); // tlačítko 9
     UG_ButtonSetFont( &window_1, BTN_ID_9, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_9, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_9, "9" );
 
     UG_ButtonCreate( &window_1, &button_plus, BTN_ID_16, 144, 48, 192, 96 ); // tlačítko +
     UG_ButtonSetFont( &window_1, BTN_ID_16, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_16, C_LIME );
     UG_ButtonSetText( &window_1, BTN_ID_16, "+" );
 
     UG_ButtonCreate( &window_1, &button_kalk, BTN_ID_21, 192, 48, 235, 144 ); // tlačítko =
     UG_ButtonSetFont( &window_1, BTN_ID_21, &FONT_24X40 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_21, C_LIME );
     UG_ButtonSetText( &window_1, BTN_ID_21, "=" );
     
     UG_ButtonCreate( &window_1, &button_4, BTN_ID_4,        0, 96, 48, 144 ); // tlačítko 4
     UG_ButtonSetFont( &window_1, BTN_ID_4, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_4, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_4, "4" );
 
     UG_ButtonCreate( &window_1, &button_5, BTN_ID_5,       48, 96, 96, 144 ); // tlačítko 5
     UG_ButtonSetFont( &window_1, BTN_ID_5, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_5, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_5, "5" );
 
     UG_ButtonCreate( &window_1, &button_6, BTN_ID_6,       96, 96, 144, 144 ); // tlačítko 6
     UG_ButtonSetFont( &window_1, BTN_ID_6, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_6, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_6, "6" );
 
     UG_ButtonCreate( &window_1, &button_minus, BTN_ID_17, 144, 96, 192, 144 ); // tlačítko -
     UG_ButtonSetFont( &window_1, BTN_ID_17, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_17, C_LIME );
     UG_ButtonSetText( &window_1, BTN_ID_17, "-" );
 
     UG_ButtonCreate( &window_1, &button_1, BTN_ID_1, 0, 144, 48, 192 ); // tlačítko 1
     UG_ButtonSetFont( &window_1, BTN_ID_1, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_1, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_1, "1" );
 
     UG_ButtonCreate( &window_1, &button_2, BTN_ID_2, 48, 144, 96, 192 ); // tlačítko 2
     UG_ButtonSetFont( &window_1, BTN_ID_2, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_2, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_2, "2" );
 
     UG_ButtonCreate( &window_1, &button_3, BTN_ID_3, 96, 144, 144, 192 ); // tlačítko 3
     UG_ButtonSetFont( &window_1, BTN_ID_3, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_3, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_3, "3" );
 
     UG_ButtonCreate( &window_1, &button_krat, BTN_ID_18, 144, 144, 192, 192 ); // tlačítko *
     UG_ButtonSetFont( &window_1, BTN_ID_18, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_18, C_LIME );
     UG_ButtonSetText( &window_1, BTN_ID_18, "*" );
 
     UG_ButtonCreate( &window_1, &button_clear, BTN_ID_22, 192, 144, 235, 239 ); // tlačítko CLR
     UG_ButtonSetFont( &window_1, BTN_ID_22, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_22, C_TOMATO );
     UG_ButtonSetText( &window_1, BTN_ID_22, "C\nL\nR" );
     
     UG_ButtonCreate( &window_1, &button_0, BTN_ID_0, 0, 192, 48, 239 ); // tlačítko 0
     UG_ButtonSetFont( &window_1, BTN_ID_0, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_0, C_LIGHT_SKY_BLUE );
     UG_ButtonSetText( &window_1, BTN_ID_0, "0" );
         
     UG_ButtonCreate( &window_1, &button_back, BTN_ID_20, 48, 192, 144, 239 ); // tlačítko BCK
     UG_ButtonSetFont( &window_1, BTN_ID_20, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_20, C_TOMATO );
     UG_ButtonSetText( &window_1, BTN_ID_20, "BCK" );
     
     UG_ButtonCreate( &window_1, &button_deleno, BTN_ID_19, 144, 192, 192, 239 ); // tlačítko /
     UG_ButtonSetFont( &window_1, BTN_ID_19, &FONT_12X20 );
     UG_ButtonSetBackColor( &window_1, BTN_ID_19, C_LIME );
     UG_ButtonSetText( &window_1, BTN_ID_19, "/" );

     UG_ButtonCreate(&window_1, &button_open_parenth, BTN_ID_23, 0, 239, 96, 286); // tlačítko (
     UG_ButtonSetFont(&window_1, BTN_ID_23, &FONT_12X20);
     UG_ButtonSetBackColor(&window_1, BTN_ID_23, C_LIME);
     UG_ButtonSetText(&window_1, BTN_ID_23, "(");
     
     UG_ButtonCreate(&window_1, &button_close_parenth, BTN_ID_24, 96, 239, 192, 286); // tlačítko )
     UG_ButtonSetFont(&window_1, BTN_ID_24, &FONT_12X20);
     UG_ButtonSetBackColor(&window_1, BTN_ID_24, C_LIME);
     UG_ButtonSetText(&window_1, BTN_ID_24, ")");

     // kreslení hlavního okna na displej
     UG_WindowShow( &window_1 );
     UG_WaitForUpdate();
 
     while (1) {
 
     }
 }
 