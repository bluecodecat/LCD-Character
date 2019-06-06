
#include "menu.h"
#include "menu_config.h"
#include "lcd_config.h"
#include <string.h>
#include <stdarg.h>
//####################################################################################################
typedef struct
{
  uint32_t  Time;
  uint8_t   Inited;
  void      (*CurrentCallback)(Menu_Key_t); 
  char      *VolatileText;
  char      Buffer[(_LCD_ROWS*_LCD_COLS)+10];
  char      ItemsText[_MENU_MAX_ITEMS][_LCD_COLS-2];  
  
  Menu_CurrentMode_t  CurrentMode;
  
  uint8_t     K_Up;
  uint8_t     K_Down;
  uint8_t     K_Back;
  uint8_t     K_Select;
  Menu_Key_t  KeyPressed; 

  uint8_t     VerMenu_Selected;    
  uint8_t     VerMenu_ItemsCnt;
  uint8_t     VerMenu_Refresh;
  
}Menu_t;

Menu_t  Menu;

extern void     LCD_Delay_ms(uint8_t  ms);
//####################################################################################################
void  Menu_Init(void)
{
  const uint8_t UpArrow[] = { 0x04,0x0E,0x15,0x04,0x04,0x04,0x04,0x00};
  const uint8_t DownArrow[] = { 0x00,0x04,0x04,0x04,0x04,0x15,0x0E,0x04};
  const uint8_t SelectArrow[] = { 0x08,0x04,0x02,0x1F,0x1F,0x02,0x04,0x08};
  LCD_CreateChar(5,(uint8_t*)UpArrow);
  LCD_CreateChar(6,(uint8_t*)DownArrow);
  LCD_CreateChar(7,(uint8_t*)SelectArrow); 
  memset(&Menu,0,sizeof(Menu));
  Menu.Time = HAL_GetTick();
  Menu.Inited = 1; 
}
//####################################################################################################
void  Menu_ReadKeys(void)
{
  if(HAL_GPIO_ReadPin(_MENU_KEY_UP_GPIO,_MENU_KEY_UP_PIN)==GPIO_PIN_RESET)
  {
    if(Menu.K_Up<255)
      Menu.K_Up++;   
    if(Menu.K_Up==5)
      Menu.KeyPressed |= Menu_Key_Up;      
  }
  else
    Menu.K_Up=0;
  if(HAL_GPIO_ReadPin(_MENU_KEY_DOWN_GPIO,_MENU_KEY_DOWN_PIN)==GPIO_PIN_RESET)
  {
    if(Menu.K_Down<255)
      Menu.K_Down++;   
    if(Menu.K_Down==5)
      Menu.KeyPressed |= Menu_Key_Down;      
  }
  else
    Menu.K_Down=0;
  if(HAL_GPIO_ReadPin(_MENU_KEY_BACK_GPIO,_MENU_KEY_BACK_PIN)==GPIO_PIN_RESET)
  {
    if(Menu.K_Back<255)
      Menu.K_Back++;   
    if(Menu.K_Back==5)
      Menu.KeyPressed |= Menu_Key_Back;      
  }
  else
    Menu.K_Back=0;
  if(HAL_GPIO_ReadPin(_MENU_KEY_SELECT_GPIO,_MENU_KEY_SELECT_PIN)==GPIO_PIN_RESET)
  {
    if(Menu.K_Select<255)
      Menu.K_Select++;   
    if(Menu.K_Select==5)
      Menu.KeyPressed |= Menu_Key_Select;      
  }
  else
    Menu.K_Select=0; 
}
//####################################################################################################
void  Menu_Loop(void)
{
  if(HAL_GetTick()-Menu.Time <= 10)
    return;
  Menu.Time = HAL_GetTick();
  if(Menu.Inited==0)
    return;  
  Menu_ReadKeys();
  switch(Menu.CurrentMode)
  {
    //########################
    case Menu_CurrentMode_None:
      
    break;
    //########################
    case Menu_CurrentMode_StaticText:
      
    break;
    //########################
    case Menu_CurrentMode_VolatileText:
      if(strcmp(Menu.Buffer,Menu.VolatileText)!=0)
      {
        memset(Menu.Buffer,0,sizeof(Menu.Buffer));
        strncpy(Menu.Buffer,Menu.VolatileText,sizeof(Menu.Buffer));
        LCD_Puts(0,0,Menu.VolatileText);
        for(uint8_t i=strlen(Menu.VolatileText);i<sizeof(Menu.Buffer);i++)
          LCD_Put(' ');
      }      
    break;    
    //########################
    case Menu_CurrentMode_ScrollingVertical:
        
      if(Menu.KeyPressed == Menu_Key_Up)
      {
        if(Menu.VerMenu_Selected > 0)
        {
          Menu.VerMenu_Selected--;
          Menu.VerMenu_Refresh=1;
        }
      }
      if(Menu.KeyPressed == Menu_Key_Down)
      {
        if (Menu.VerMenu_Selected<Menu.VerMenu_ItemsCnt-1)
        {
          Menu.VerMenu_Selected++;
          Menu.VerMenu_Refresh=1;
        }        
      }    
      if(Menu.VerMenu_Refresh == 1)
      {
        Menu.VerMenu_Refresh=0;
        uint8_t p=Menu.VerMenu_Selected/_LCD_ROWS;
        uint8_t s=Menu.VerMenu_Selected%_LCD_ROWS;
        for(uint8_t r=0; r<_LCD_ROWS ; r++)
        {
          for(uint8_t i=1;i<_LCD_COLS-1 ; i++)
            LCD_Puts(i,r," ");          
          LCD_Puts(1,r,Menu.ItemsText[p*_LCD_ROWS+r]);          
        }
        if(p>0)
          LCD_PutCustom(_LCD_COLS-1,0,5);//up arrow
        else
          LCD_Puts(_LCD_COLS-1,0," ");
        if(Menu.VerMenu_Selected<Menu.VerMenu_ItemsCnt-1)
          LCD_PutCustom(_LCD_COLS-1,_LCD_ROWS-1,6);//down arrow
        else
          LCD_Puts(_LCD_COLS-1,_LCD_ROWS-1," ");
        for(uint8_t i=0;i<_LCD_ROWS;i++)
        {
          if(i==s)
            LCD_PutCustom(0,i,7);//select arrow  
          else
            LCD_Puts(0,i," ");
        }        
      }      
    break;
    //########################
  }  
  if(Menu.CurrentCallback!=NULL)
    Menu.CurrentCallback(Menu.KeyPressed);    
  if(Menu.KeyPressed != Menu_Key_None)
  {
    #if (_MENU_BEEP_ENABLE==1)
    HAL_GPIO_WritePin(_MENU_BEEP_GPIO,_MENU_BEEP_PIN,GPIO_PIN_SET);
    LCD_Delay_ms(20);
    HAL_GPIO_WritePin(_MENU_BEEP_GPIO,_MENU_BEEP_PIN,GPIO_PIN_RESET);
    #endif
    Menu.KeyPressed = Menu_Key_None;    
  }
}
//####################################################################################################
void  Menu_DeleteCurrent(void)
{
  Menu.CurrentCallback = NULL;  
  LCD_Clear();
  Menu.CurrentMode = Menu_CurrentMode_None;
}
//####################################################################################################
void  Menu_CreateStaticText(void callback(Menu_Key_t),const char *text)
{
  LCD_Clear();
  LCD_Puts(0,0,(char*)text);
  Menu.CurrentMode = Menu_CurrentMode_StaticText; 
  Menu.CurrentCallback = callback;
}
//####################################################################################################
void  Menu_CreateVolatileText(void callback(Menu_Key_t),char *text)
{
  LCD_Clear();
  Menu.VolatileText = text;
  LCD_Puts(0,0,Menu.VolatileText);
  memset(Menu.Buffer,0,sizeof(Menu.Buffer));
  strncpy(Menu.Buffer,text,sizeof(Menu.Buffer));
  Menu.CurrentMode = Menu_CurrentMode_VolatileText; 
  Menu.CurrentCallback = callback;  
}
//####################################################################################################
void  Menu_CreateScrollingVertical(void callback(Menu_Key_t),uint8_t ItemsCnt,...)
{
  va_list tag;
  va_start (tag,ItemsCnt);
  char *arg[_MENU_MAX_ITEMS];
  memset(arg,0,sizeof(arg));
  for(uint8_t i=0; i<ItemsCnt ; i++)
    arg[i] = va_arg (tag, char*);  
  memset(Menu.ItemsText,0,sizeof(Menu.ItemsText));
  for(uint8_t i=0; i<ItemsCnt ; i++)
    strncpy(Menu.ItemsText[i],arg[i],_LCD_COLS-2);
  va_end (tag);  
  Menu.VerMenu_ItemsCnt = ItemsCnt;
  Menu.VerMenu_Selected = 0;
  Menu.VerMenu_Refresh = 1;
  memset(Menu.Buffer,0,sizeof(Menu.Buffer));
  Menu.CurrentMode = Menu_CurrentMode_ScrollingVertical; 
  Menu.CurrentCallback = callback;  
}
//####################################################################################################
uint8_t Menu_GetScrollingSelected(void)
{
  return Menu.VerMenu_Selected; 
}
//####################################################################################################