
#define TextOutC2(x_start, x_end, y, string,  z) textCpixel((x_start)>>4,(x_end)>>4,((y)>>4)+4,GS_SET_RGBA(255,255,255,128),0,0,(z),(string))

void printch(int x, int y, unsigned couleur,unsigned char ch,int taille,int pl,int zde);
void textpixel(int x,int y,unsigned color,int tail,int plein,int zdep, char *string,...);
void textCpixel(int x,int x2,int y,unsigned color,int tail,int plein,int zdep,char *string,...);
