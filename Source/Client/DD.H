extern MAXX,MAXY;
extern unsigned int volatile dd_tick;

int dd_init(HWND hwnd,int x,int y);
void dd_init_sprites();
int dd_change(int x,int y);
void dd_savescreen(void);
void dd_ticker(void);
void dd_init_sprites(void);
void restore_palette(void);
int dd_isvisible(void);
void dd_deinit(void);
void dd_shadow_clear(void);
void dd_shadow(int nr,int xpos,int ypos,int xoff,int yoff);
