/***************************************************/
/* This include file defines a macros used to      */
/* manipulate events in X-11.                      */
/*           Author: Michael D. Garris 3/23/89     */
/***************************************************/

#define XMButton(event)  (((XButtonEvent *)&event)->button)

#define XMGetButton(display,event,button) \
do{ \
   XNextEvent(display,&event); \
}while(XMButton(event) != (button));

#define XMGetEvent(display,event,_type) \
do{ \
   XNextEvent(display,&event); \
}while(event.type != (_type));

