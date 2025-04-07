#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
extern "C"
{
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}
using namespace std;

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define MIN_WIDTH 0
#define MAX_WIDTH 640

#define blokowoanie_z_lewej_dla_distance -2
#define blokowoanie_z_prawej_dla_distance  2

#define predkosc_gracza 1.5

#define poziom0 (SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3) +58
#define gracz_pocz SCREEN_HEIGHT/2+SCREEN_HEIGHT/3
#define dlugosc_drabiny 25

#define granica_pietro_0 1
#define granica_pietro_1 0.440
#define granica_pietro_2 -0.120
#define granica_pietro_3 -0.680

#define wysokosc_skoku 1.2
#define grawitacja 4
#define ciezar_poczatkowy 1

#define wysokosc_double_jump 2.2
#define odleglosc_serc 20
#define predkosc_beczki 1.5
#define odleglosc_miedzy_beczkami -3
struct drzwi
{
	int position_x;
	int position_y;
	int hitbox_left;
	int hitbox_right;
	int hitbox_top;
	int hitbox_bottom;
};
struct pietra
{
	double pusty_przedzial_1;
	double pusty_przedzial_2;
	int gorna_granica;
};
struct beczka
{
	int czy_zebral_punkty = 0;
	double distance_beczki = 0;
	double distance_y_beczki = 1;
	double zapamietane_distance_beczki = 0;
	double zapamietane_distance_y_beczki = 1;
	double predkosc_beczki_x = predkosc_beczki;
	double predkosc_beczki_y = 0;
	double predkosc_spadku_beczki = 1.2;
	int flaga_spadanie_beczki = 0;
	int spadanie_beczki_ktore_pietro = -1;
	int pietro_beczki = -1;
	int beczka_position_x;
	int beczka_position_y;
	int hitbox_beczki_x_lewo;
	int hitbox_beczki_x_prawo;
	int hitbox_beczki_y_dolny;
	int hitbox_beczki_y_gorny;
	int czy_gracz_ominal_beczke = 1;
};
struct moneta
{
	int czy_nadal_jest = 1;
	int hitbox_moneta_x_lewo;
	int hitbox_moneta_x_prawo;
	int hitbox_moneta_y_dolny;
	int hitbox_moneta_y_gorny;
	int position_monety_x;
	int position_monety_y;
};

struct mario
{
	double distance=0;
	double distance_y=1;
	double playerspeed;
	double playerspeed_y;
	int czy_lokalizacja_drabiny = 0;
	int pietro = 0;
	int flaga_czy_strata_hp = 0;
	int flaga_czy_w_gore = 0;
	int flaga_czy_w_dol = 0;
	int czy_w_gore_animacja = 0;
	int czy_w_prawo_animacja = 0;
	int czy_w_lewo_animacja = 0;
	double predkosc_spadku = 1;
	int czy_skok = 0;
	double predkosc_skoku = 0.0;
	int flaga_spadanie = 0;
	int spadanie_ktore_pietro = -1;
	int czy_jest_na_drabinie = 0;
	int czy_double_jump = 0;
	int zycie = 3;
	int punkty = 0;
	int mario_position_x;
	int mario_position_y;
	int czy_przeskoczyl_beczke = 0;
	int czy_przeszedl_poziom = 0;
};
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset)
{
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text)
	{
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	}
}
// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y)
{
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
}
// rysowanie pojedynczego pixela
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color)
{
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
}
// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color)
{
	for (int i = 0; i < l; i++)
	{
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	}
}
// rysowanie prostok¹ta o d³ugoœci boków l i k
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor)
{
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
}
void animacje(double klatki, mario& mario, SDL_Surface*& player, SDL_Surface*& beczka, SDL_Surface*& kong, struct beczka beczki[10], SDL_Surface* screen, SDL_Surface* charset, char text[128],int flaga_menu)
{
	//chodzenie mario animacja
	if (mario.czy_w_prawo_animacja == 1 && mario.czy_skok == 0)
	{
		if (klatki >= 0.25)
		{
			player = SDL_LoadBMP("./mario_right.bmp");
		}
		else
			player = SDL_LoadBMP("./mario_right2.bmp");
	}
	if (mario.czy_w_lewo_animacja == 1 && mario.czy_skok == 0)
	{
		if (klatki >= 0.25)
		{
			player = SDL_LoadBMP("./mario_left.bmp");
		}
		else
			player = SDL_LoadBMP("./mario_left2.bmp");
	}
	if (mario.czy_w_gore_animacja == 1)
	{
		if (klatki >= 0.25)
		{
			player = SDL_LoadBMP("./climb_left.bmp");
		}
		else
			player = SDL_LoadBMP("./climb_right.bmp");
	}

	//beczka animacja
	if (klatki <= 0.125)
		beczka = SDL_LoadBMP("./beczka1.bmp");
	else if (klatki <= 0.25)
		beczka = SDL_LoadBMP("./beczka2.bmp");
	else if (klatki <= 0.375)
		beczka = SDL_LoadBMP("./beczka3.bmp");
	else
		beczka = SDL_LoadBMP("./beczka4.bmp");

	//krecenie beczki
	for (int i = 1; i <= 2; i++)
	{
		if (beczki[i].beczka_position_x >= SCREEN_WIDTH - 110 && beczki[i].pietro_beczki == 0)
			kong = SDL_LoadBMP("./donkey3.bmp");
		if (beczki[i].beczka_position_x == SCREEN_WIDTH / 7 + 30 && beczki[i].pietro_beczki == 2)
			kong = SDL_LoadBMP("./donkey2.bmp");
		if (beczki[i].beczka_position_x == SCREEN_WIDTH / 7 + 100 && beczki[i].pietro_beczki == 2)
			kong = SDL_LoadBMP("./donkey.bmp");
		if (beczki[i].beczka_position_x == SCREEN_WIDTH / 7 - 75 && beczki[i].pietro_beczki == 2)
		{
			if (flaga_menu == 1)
			kong = SDL_LoadBMP("./donkey4.bmp");
			else if(flaga_menu==2)
			kong = SDL_LoadBMP("./donkey4_1.bmp");
			else
			kong = SDL_LoadBMP("./donkey4_2.bmp");
		}
	}
	
}
void logika_beczki(double deltatime, struct beczka beczki[], int liczba_beczek, struct pietra podstawy_lvl1[]) 
{
	for (int i = 1; i <= liczba_beczek; i++)
	{

		beczki[i].distance_beczki += beczki[i].predkosc_beczki_x * deltatime;
		beczki[i].distance_y_beczki += beczki[i].predkosc_beczki_y * deltatime;
		beczki[i].beczka_position_x = beczki[i].distance_beczki * SCREEN_HEIGHT / 3;
		beczki[i].beczka_position_y = 70 + beczki[i].distance_y_beczki * SCREEN_HEIGHT / 3;
		beczki[i].hitbox_beczki_x_lewo = beczki[i].beczka_position_x - 12;
		beczki[i].hitbox_beczki_x_prawo = beczki[i].beczka_position_x + 12;
		beczki[i].hitbox_beczki_y_gorny = beczki[i].beczka_position_y - 11;
		beczki[i].hitbox_beczki_y_dolny = beczki[i].beczka_position_y + 11;
		//spadanie beczki
		if (beczki[i].pietro_beczki == 1 && beczki[i].beczka_position_x < podstawy_lvl1[1].pusty_przedzial_2)
		{
			beczki[i].flaga_spadanie_beczki = 1;
			beczki[i].spadanie_beczki_ktore_pietro = 1;
		}
		if (beczki[i].pietro_beczki == 2 && beczki[i].beczka_position_x > podstawy_lvl1[2].pusty_przedzial_1)
		{
			beczki[i].flaga_spadanie_beczki = 1;
			beczki[i].spadanie_beczki_ktore_pietro = 2;
		}
		if (beczki[i].flaga_spadanie_beczki == 1)
		{
			beczki[i].distance_y_beczki += beczki[i].predkosc_spadku_beczki * deltatime;
			beczki[i].predkosc_spadku_beczki += grawitacja * deltatime;
			if (beczki[i].beczka_position_y >= podstawy_lvl1[0].gorna_granica + 10 && beczki[i].pietro_beczki == 0 && beczki[i].spadanie_beczki_ktore_pietro == 1)
			{
				beczki[i].predkosc_spadku_beczki = ciezar_poczatkowy;
				beczki[i].flaga_spadanie_beczki = 0;
				beczki[i].spadanie_beczki_ktore_pietro = -1;

			}
			if (beczki[i].beczka_position_y >= podstawy_lvl1[1].gorna_granica + 10 && beczki[i].pietro_beczki == 1 && beczki[i].spadanie_beczki_ktore_pietro == 2)
			{
				beczki[i].predkosc_spadku_beczki = ciezar_poczatkowy;
				beczki[i].flaga_spadanie_beczki = 0;
				beczki[i].spadanie_beczki_ktore_pietro = -1;
			}
		}
		if (beczki[i].pietro_beczki == 0 && beczki[i].beczka_position_x >= MAX_WIDTH && beczki[i].spadanie_beczki_ktore_pietro == -1)
		{
			beczki[i].distance_beczki = -0.1;
			beczki[i].distance_y_beczki = 1;
		}
		if (beczki[i].pietro_beczki == 2 && beczki[i].beczka_position_x <= MIN_WIDTH)
			beczki[i].predkosc_beczki_x = predkosc_beczki;

		else if (beczki[i].beczka_position_x >= MAX_WIDTH)
		{
			beczki[i].distance_beczki -= 0.1;
			beczki[i].predkosc_beczki_x *= (-1);
		}
		else if (beczki[i].beczka_position_x <= MIN_WIDTH)
		{
			beczki[i].distance_beczki += 0.05;
			beczki[i].predkosc_beczki_x *= (-1);
		}
	}


}
void ograniczanie_mapy(mario& mario, const double blokowanie_z_lewej, const double blokowanie_z_prawej) 
{
	if (mario.distance <= blokowanie_z_lewej)
		mario.distance = blokowanie_z_lewej;
	if (mario.distance >= blokowanie_z_prawej)
		mario.distance = blokowanie_z_prawej;
}
void logika_drabiny(mario& mario, const int lokalizacja_drabin_lvl1[][20], const pietra podstawy_lvl1[]) {
	//drabina piertro 0 
	if (mario.distance_y >= granica_pietro_0)
		mario.distance_y = granica_pietro_0;

	if (mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][0] && mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][1] &&
		mario.mario_position_y <= podstawy_lvl1[1].gorna_granica && mario.flaga_czy_w_gore == 1)
	{
		mario.distance_y = granica_pietro_1;
	}
	//drabina piertro 1
	if (mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][2] && mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][3] &&
		mario.mario_position_y <= podstawy_lvl1[2].gorna_granica && mario.flaga_czy_w_gore == 1)
	{
		mario.distance_y = granica_pietro_2;
	}
	if (mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][2] && mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][3] &&
		mario.mario_position_y >= podstawy_lvl1[1].gorna_granica && mario.flaga_czy_w_dol == 1)
	{

		mario.distance_y = granica_pietro_1;
	}
	//drabina pietro 2
	if (mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][4] && mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][5] &&
		mario.mario_position_y <= podstawy_lvl1[3].gorna_granica && mario.flaga_czy_w_gore == 1)
	{
		mario.distance_y = granica_pietro_3;
	}
	if (mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][4]&& mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][5] &&
		mario.mario_position_y >= podstawy_lvl1[2].gorna_granica && mario.flaga_czy_w_dol == 1)
	{

		mario.distance_y = granica_pietro_2;
	}

}
void skakanie_na_wyzsza_platforme(mario& mario, const pietra podstawy_lvl1[]) {
	if ((mario.pietro == 0 && mario.mario_position_x < podstawy_lvl1[1].pusty_przedzial_2) ||
		(mario.pietro == 1 && mario.mario_position_x > podstawy_lvl1[2].pusty_przedzial_1) ||
		(mario.pietro == 2 && mario.mario_position_x < podstawy_lvl1[3].pusty_przedzial_2))
	{
		mario.czy_double_jump = 1;
	}
	else
		mario.czy_double_jump = 0;
}
void spadanie(mario& mario, const pietra podstawy_lvl1[], const double deltatime) {
	if (mario.pietro == 1 && mario.mario_position_x < podstawy_lvl1[1].pusty_przedzial_2)
	{
		mario.flaga_spadanie = 1;
		mario.spadanie_ktore_pietro = 1;
	}
	if (mario.pietro == 2 && mario.mario_position_x > podstawy_lvl1[2].pusty_przedzial_1)
	{
		mario.flaga_spadanie = 1;
		mario.spadanie_ktore_pietro = 2;
	}
	if (mario.flaga_spadanie == 1)
	{
		mario.distance_y += mario.predkosc_spadku * deltatime;
		mario.predkosc_spadku += grawitacja * deltatime;
		if (mario.distance_y >= granica_pietro_0 && mario.pietro == 0 && mario.spadanie_ktore_pietro == 1)
		{
			mario.predkosc_spadku = ciezar_poczatkowy;
			mario.distance_y = granica_pietro_0;
			mario.flaga_spadanie = 0;
			mario.spadanie_ktore_pietro = -1;

		}
		if (mario.distance_y >= granica_pietro_1 && mario.pietro == 1 && mario.spadanie_ktore_pietro == 2)
		{
			mario.predkosc_spadku = ciezar_poczatkowy;
			mario.distance_y = granica_pietro_1;
			mario.flaga_spadanie = 0;
			mario.spadanie_ktore_pietro = -1;
		}
	}
}
void skakanie(mario& mario, const double deltatime) 
{
	if (mario.czy_skok) {
		mario.distance_y += mario.predkosc_skoku * deltatime;
		mario.predkosc_skoku += grawitacja * deltatime;

		// Przypadek dla piêtra 0
		if (mario.distance_y >= 1 && mario.pietro == 0) {
			mario.distance_y = 1;
			mario.czy_skok = 0;
		}
		// Przypadek dla piêtra 1
		else if (mario.distance_y > granica_pietro_1 && mario.pietro == 1) {
			mario.distance_y = granica_pietro_1;
			mario.czy_skok = 0;
		}
		// Przypadek dla piêtra 2
		else if (mario.distance_y > granica_pietro_2 && mario.pietro == 2) {
			mario.distance_y = granica_pietro_2;
			mario.czy_skok = 0;
		}
		// Przypadek dla piêtra 2
		else if (mario.distance_y > granica_pietro_3 && mario.pietro == 3) {
			mario.distance_y = granica_pietro_3;
			mario.czy_skok = 0;
		}
	}
}
void sprawdzanie_kolizji_mario_z_beczka(mario& mario, beczka beczki[])
{
	for (int i = 1; i <= 2; i++)
	{
		if (mario.mario_position_x <= beczki[i].hitbox_beczki_x_prawo && mario.mario_position_x >= beczki[i].hitbox_beczki_x_lewo && mario.mario_position_y <= beczki[i].hitbox_beczki_y_dolny &&
			mario.mario_position_y >= beczki[i].hitbox_beczki_y_gorny && beczki[i].czy_gracz_ominal_beczke == 1)
		{
			beczki[i].czy_gracz_ominal_beczke = 0;
			mario.zycie--;
			mario.flaga_czy_strata_hp = 1;
		}
		if (mario.mario_position_x > beczki[i].hitbox_beczki_x_prawo || mario.mario_position_x < beczki[i].hitbox_beczki_x_lewo)
		{
			beczki[i].czy_gracz_ominal_beczke = 1;
		}
	}
}
void porazka(int& flaga_menu, mario& mario) 
{
	if (mario.zycie == 0)
	{
		flaga_menu = 0;
		mario.zycie = 3;
	}
}
void reset(mario& mario, beczka beczki[], double &worldtime)
{
	mario.distance = 0;
	mario.distance_y = 1;
	worldtime = 0;
	//RESET PO£O¯ENIA BECZEK
	for (int i = 1; i <= 2; i++)
	{
		if (i == 1)
		{
			beczki[i].distance_beczki = 0;
			beczki[i].distance_y_beczki = 1;
		}
		if (i == 2)
		{
			beczki[i].distance_beczki = odleglosc_miedzy_beczkami;
			beczki[i].distance_y_beczki = 1;
		}
		beczki[i].flaga_spadanie_beczki = 0;
		if (beczki[i].pietro_beczki == 1)
			beczki[i].pietro_beczki = 2;
	}
	mario.czy_przeszedl_poziom = 0;
}
void sprawdzanie_na_ktorym_pietrze_jest_gracz(mario& mario, const pietra podstawy_lvl1[]) {
	if (mario.mario_position_y <= podstawy_lvl1[3].gorna_granica) 
	{
		mario.pietro = 3;
	}
	else if (mario.mario_position_y <= podstawy_lvl1[2].gorna_granica) {
		mario.pietro = 2;
	}
	else if (mario.mario_position_y <= podstawy_lvl1[1].gorna_granica) {
		mario.pietro = 1;
	}
	else if (mario.mario_position_y <= podstawy_lvl1[0].gorna_granica) {
		mario.pietro = 0;
	}
	else {
		mario.pietro = -1;
	}
}
void sprawdzanie_na_ktorym_pietrze_jest_beczka(beczka beczki[], const pietra podstawy_lvl1[]) {
	for (int i = 1; i <= 2; i++) {
		if (beczki[i].beczka_position_y <= podstawy_lvl1[2].gorna_granica)
			beczki[i].pietro_beczki = 2;
		else if (beczki[i].beczka_position_y <= podstawy_lvl1[1].gorna_granica)
			beczki[i].pietro_beczki = 1;
		else if (beczki[i].beczka_position_y <= podstawy_lvl1[0].gorna_granica)
			beczki[i].pietro_beczki = 0;
	}
}
void punkty_za_skok_nad_beczka(mario& mario, beczka beczki[], SDL_Surface* screen, SDL_Surface* charset, char* text) {
	for (int i = 1; i <= 2; i++) {
		if (mario.mario_position_x > beczki[i].hitbox_beczki_x_lewo && mario.mario_position_x < beczki[i].hitbox_beczki_x_prawo && beczki[i].czy_zebral_punkty == 0) {
			if (mario.mario_position_y > beczki[i].hitbox_beczki_y_dolny - 90 && mario.mario_position_y < beczki[i].hitbox_beczki_y_dolny && mario.flaga_czy_strata_hp == 0) {
				beczki[i].czy_zebral_punkty = 1;
				mario.punkty += 50;
				mario.czy_przeskoczyl_beczke = 1;
			}
		}
		if (beczki[i].czy_zebral_punkty == 1) {
			if (mario.mario_position_x > beczki[i].hitbox_beczki_x_prawo || mario.mario_position_x < beczki[i].hitbox_beczki_x_lewo) {
				mario.czy_przeskoczyl_beczke = 0;
				beczki[i].czy_zebral_punkty = 0;
			}
		}
	}
	if (mario.czy_przeskoczyl_beczke == 1 && mario.flaga_czy_strata_hp == 0) {
		sprintf(text, "50");
		DrawString(screen, mario.mario_position_x - 10, mario.mario_position_y - 30, text, charset);
	}
}
void punkty_za_monete_i_przejscie_poziomu(mario& mario, moneta monety[], drzwi& drzwi, int& pozycja_pomocnicza, int& flaga_menu, SDL_Surface* screen, SDL_Surface* charset, char* text, beczka beczki[], double& worldtime) {
	// Punkty za monete
	if (mario.mario_position_x > monety[1].hitbox_moneta_x_lewo && mario.mario_position_x<monety[1].hitbox_moneta_x_prawo &&
		mario.mario_position_y>monety[1].hitbox_moneta_y_gorny && mario.mario_position_y < monety[1].hitbox_moneta_y_dolny && monety[1].czy_nadal_jest)
	{
		monety[1].czy_nadal_jest = 0;
		mario.punkty += 100;
		pozycja_pomocnicza = mario.mario_position_x;
	}
	if (pozycja_pomocnicza != -10)
	{
		sprintf(text, "100");
		DrawString(screen, mario.mario_position_x - 10, mario.mario_position_y - 30, text, charset);
	}
	if (pozycja_pomocnicza - 20 > mario.mario_position_x || mario.mario_position_x > pozycja_pomocnicza + 20)
		pozycja_pomocnicza = -10;
	//sprawdzanie czy przeszedl poziom
	if (mario.mario_position_x > drzwi.hitbox_left && mario.mario_position_x<drzwi.hitbox_right &&
		mario.mario_position_y>drzwi.hitbox_top && mario.mario_position_y < drzwi.hitbox_bottom)
	{
		mario.czy_skok =0;
		mario.czy_przeszedl_poziom = 1;
		mario.punkty += 500;
		if (flaga_menu <3)
		{
			monety[1].czy_nadal_jest = 1;
			flaga_menu++;
			reset(mario, beczki, worldtime);
		}
		else
			flaga_menu = 0;
	}
}
void wyswietlanie_zycia(SDL_Surface* screen, SDL_Surface* serce, mario& mario)
{
	for (int i = 0; i < mario.zycie; i++)
	{
		if (i == 0)
			DrawSurface(screen, serce,
				6 * SCREEN_WIDTH / 7,
				SCREEN_HEIGHT / 6);
		if (i == 1)
			DrawSurface(screen, serce,
				(6 * SCREEN_WIDTH / 7) + odleglosc_serc,
				SCREEN_HEIGHT / 6);
		if (i == 2)
			DrawSurface(screen, serce,
				6 * SCREEN_WIDTH / 7 + odleglosc_serc * 2,
				SCREEN_HEIGHT / 6);
	}
}
void menu_gry(SDL_Surface* screen, SDL_Surface* charset, int czerwony, int niebieski, double worldTime, char text[]) 
{
	DrawRectangle(screen, 4, 4, SCREEN_WIDTH - 8, 54, czerwony, niebieski);

	sprintf(text, "JEDRZEJ JASINIECKI nr_indeksu:197993 ");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

	sprintf(text, "czas trwania = % .1lf s, n - nowa gra  ", worldTime);
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);

	sprintf(text, "Esc - wyjscie, \032  \033 \030 \031 - sterowanie , spacja - skok");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 42, text, charset);
}
void kontynuacja_po_stracie_zycia(SDL_Surface* screen, SDL_Surface* charset, int niebieski, int czerwony, mario& mario, int& flaga_menu, char text[], moneta monety[]) {
	if (mario.flaga_czy_strata_hp == 1 && mario.zycie > 0 && flaga_menu != 0)
	{
		DrawRectangle(screen, SCREEN_WIDTH / 4 - 25, 200, SCREEN_WIDTH / 2 + 50, 54, niebieski, czerwony);

		sprintf(text, "Esc - wyjdz z gry, k-kontynuuj, wynik = %d ", mario.punkty);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 220, text, charset);
		monety[1].czy_nadal_jest = 1;
	}
}
void koniec_zycia(SDL_Surface* screen, SDL_Surface* charset, int niebieski, int czerwony, mario& mario, int& flaga_menu, char text[], moneta monety[]) {
	if (flaga_menu == 0)
	{
		DrawRectangle(screen, SCREEN_WIDTH / 4, 200, SCREEN_WIDTH / 2, 54, niebieski, czerwony);

		sprintf(text, "zapisac gre? k-tak p-nie ");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 220, text, charset);
		if (!mario.czy_przeszedl_poziom)
			mario.punkty = 0;
		monety[1].czy_nadal_jest = 1;
	}
}
void sprawdzanie_czy_moze_wejsc_na_drabine(mario& mario, const int lokalizacja_drabin_lvl1[][20])
{
	//drabina na parterze - level 1
	if (mario.distance_y >= granica_pietro_1 && mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][0] &&
		mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][1])
		mario.czy_lokalizacja_drabiny = 1;
	//drabina na 1 pietrze - level 1
	else if (mario.distance_y >= granica_pietro_2 && mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][2] &&
		mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][3])
		mario.czy_lokalizacja_drabiny = 1;

	//drabina na 2 pietrze
	else if (mario.distance_y >= granica_pietro_3 && mario.mario_position_x >= lokalizacja_drabin_lvl1[mario.pietro][4] &&
		mario.mario_position_x <= lokalizacja_drabin_lvl1[mario.pietro][5])
		mario.czy_lokalizacja_drabiny = 1;
	else
		mario.czy_lokalizacja_drabiny = 0;
}
void mapa_level_pierwszy(SDL_Surface* screen, SDL_Surface* dirt, SDL_Surface* wall, SDL_Surface* ladder, SDL_Surface* coin, SDL_Surface* player,
	SDL_Surface* beczka, SDL_Surface* kong, struct mario& mario, struct pietra podstawy_lvl1[], struct beczka beczki[], struct moneta monety[],
	int lokalizacja_drabin_lvl1[][20],int czarny, SDL_Surface* door,struct drzwi& drzwi)
{
	SDL_FillRect(screen, NULL, czarny);
	//pietro 0
	DrawSurface(screen, dirt,
		SCREEN_WIDTH / 2,
		poziom0);
	podstawy_lvl1[0].gorna_granica = 400;
	//pietro 1
	DrawSurface(screen, wall,
		SCREEN_WIDTH / 1.5,
		poziom0 - 120);
	podstawy_lvl1[1].pusty_przedzial_1 = 0;
	podstawy_lvl1[1].pusty_przedzial_2 = 170;
	podstawy_lvl1[1].gorna_granica = 310;
	//pietro 2
	DrawSurface(screen, wall,
		SCREEN_WIDTH / 3,
		poziom0 - 210);
	podstawy_lvl1[2].pusty_przedzial_1 = 470;
	podstawy_lvl1[2].pusty_przedzial_2 = 680;
	podstawy_lvl1[2].gorna_granica = 220;
	//drabina pietro 0-1
	DrawSurface(screen, ladder,
		SCREEN_WIDTH / 3,
		poziom0 - 120 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[0][0] = 200;
	lokalizacja_drabin_lvl1[0][1] = 235;
	lokalizacja_drabin_lvl1[1][0] = 200;
	lokalizacja_drabin_lvl1[1][1] = 235;
	//drabina pietro 1-2
	DrawSurface(screen, ladder,
		3 * SCREEN_WIDTH / 5,
		poziom0 - 210 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[1][2] = 370;
	lokalizacja_drabin_lvl1[1][3] = 400;
	lokalizacja_drabin_lvl1[2][2] = 370;
	lokalizacja_drabin_lvl1[2][3] = 400;

	if (monety[1].czy_nadal_jest)
	{
		DrawSurface(screen, coin,
			SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2,
			SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3);
		monety[1].hitbox_moneta_x_lewo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 - 10;
		monety[1].hitbox_moneta_x_prawo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 + 10;
		monety[1].hitbox_moneta_y_dolny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 + 10;
		monety[1].hitbox_moneta_y_gorny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 - 10;
	}
	DrawSurface(screen, door,
		SCREEN_WIDTH/3,
		 SCREEN_HEIGHT / 2-30);
	drzwi.position_x = SCREEN_WIDTH / 3;
	drzwi.position_y = SCREEN_HEIGHT / 2 - 30;
	drzwi.hitbox_bottom = SCREEN_HEIGHT / 2 - 30 + 30;
	drzwi.hitbox_top = SCREEN_HEIGHT / 2 - 30 - 30;
	drzwi.hitbox_left = SCREEN_WIDTH / 3 - 30;
	drzwi.hitbox_right = SCREEN_WIDTH / 3 + 30;
	//koniec rysowania mapa poziom 1
	//gracz//
	DrawSurface(screen, player,
		SCREEN_WIDTH / 2 + mario.distance * SCREEN_HEIGHT / 3,
		SCREEN_HEIGHT / 2 + mario.distance_y * SCREEN_HEIGHT / 3);

	//beczki//
	DrawSurface(screen, beczka,
		beczki[1].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[1].distance_y_beczki * SCREEN_HEIGHT / 3);
	DrawSurface(screen, beczka,
		beczki[2].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[2].distance_y_beczki * SCREEN_HEIGHT / 3);

	//donkey kong
	DrawSurface(screen, kong,
		SCREEN_HEIGHT / 7,
		45 + SCREEN_HEIGHT / 3);
}
void mapa_level_drugi(SDL_Surface* screen, SDL_Surface* podstawa, SDL_Surface* platforma, SDL_Surface* ladder, SDL_Surface* coin, SDL_Surface* player,
	SDL_Surface* beczka, SDL_Surface* kong, struct mario& mario, struct pietra podstawy_lvl1[], struct beczka beczki[], struct moneta monety[],
	int lokalizacja_drabin_lvl1[][20], int ciemnyczerwony, SDL_Surface* door, struct drzwi& drzwi)
{
	SDL_FillRect(screen, NULL, ciemnyczerwony);
	//pietro 0
	DrawSurface(screen, podstawa,
		SCREEN_WIDTH / 2,
		poziom0);
	podstawy_lvl1[0].gorna_granica = 400;
	//pietro 1
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 1.5,
		poziom0 - 120);
	podstawy_lvl1[1].pusty_przedzial_1 = 0;
	podstawy_lvl1[1].pusty_przedzial_2 = 170;
	podstawy_lvl1[1].gorna_granica = 310;
	//pietro 2
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 3,
		poziom0 - 210);
	podstawy_lvl1[2].pusty_przedzial_1 = 470;
	podstawy_lvl1[2].pusty_przedzial_2 = 680;
	podstawy_lvl1[2].gorna_granica = 220;
	//pietro 3
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 1.5,
		poziom0 - 300);
	podstawy_lvl1[3].pusty_przedzial_1 = 0;
	podstawy_lvl1[3].pusty_przedzial_2 = 170;
	podstawy_lvl1[3].gorna_granica = 130;

	//drabina pietro 0-1
	DrawSurface(screen, ladder,
		SCREEN_WIDTH / 3,
		poziom0 - 120 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[0][0] = 200;
	lokalizacja_drabin_lvl1[0][1] = 235;
	lokalizacja_drabin_lvl1[1][0] = 200;
	lokalizacja_drabin_lvl1[1][1] = 235;
	//drabina pietro 1-2
	DrawSurface(screen, ladder,
		3 * SCREEN_WIDTH / 5,
		poziom0 - 210 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[1][2] = 370;
	lokalizacja_drabin_lvl1[1][3] = 400;
	lokalizacja_drabin_lvl1[2][2] = 370;
	lokalizacja_drabin_lvl1[2][3] = 400;

	if (monety[1].czy_nadal_jest)
	{
		DrawSurface(screen, coin,
			SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2,
			SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3);
		monety[1].hitbox_moneta_x_lewo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 - 10;
		monety[1].hitbox_moneta_x_prawo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 + 10;
		monety[1].hitbox_moneta_y_dolny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 + 10;
		monety[1].hitbox_moneta_y_gorny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 - 10;
	}
	DrawSurface(screen, door,
		SCREEN_WIDTH / 3,
		SCREEN_HEIGHT / 2 - 30-90);
	drzwi.position_x = SCREEN_WIDTH / 3;
	drzwi.position_y = SCREEN_HEIGHT / 2 - 30-90;
	drzwi.hitbox_bottom = SCREEN_HEIGHT / 2 - 30 + 30-90;
	drzwi.hitbox_top = SCREEN_HEIGHT / 2 - 30 - 30-90;
	drzwi.hitbox_left = SCREEN_WIDTH / 3 - 30;
	drzwi.hitbox_right = SCREEN_WIDTH / 3 + 30;
	//koniec rysowania mapa poziom 1
	//gracz//
	DrawSurface(screen, player,
		SCREEN_WIDTH / 2 + mario.distance * SCREEN_HEIGHT / 3,
		SCREEN_HEIGHT / 2 + mario.distance_y * SCREEN_HEIGHT / 3);

	//beczki//
	DrawSurface(screen, beczka,
		beczki[1].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[1].distance_y_beczki * SCREEN_HEIGHT / 3);
	DrawSurface(screen, beczka,
		beczki[2].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[2].distance_y_beczki * SCREEN_HEIGHT / 3);

	//donkey kong
	DrawSurface(screen, kong,
		SCREEN_HEIGHT / 7,
		45 + SCREEN_HEIGHT / 3);
}
void mapa_level_trzeci(SDL_Surface* screen, SDL_Surface* podstawa, SDL_Surface* platforma, SDL_Surface* ladder, SDL_Surface* coin, SDL_Surface* player,
	SDL_Surface* beczka, SDL_Surface* kong, struct mario& mario, struct pietra podstawy_lvl1[], struct beczka beczki[], struct moneta monety[],
	int lokalizacja_drabin_lvl1[][20], int jasnoniebieski, SDL_Surface* door, struct drzwi& drzwi, SDL_Surface* platforma2)
{
	SDL_FillRect(screen, NULL, jasnoniebieski);
	//pietro 0
	DrawSurface(screen, podstawa,
		SCREEN_WIDTH / 2,
		poziom0);
	podstawy_lvl1[0].gorna_granica = 400;
	//pietro 1
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 1.5,
		poziom0 - 120);
	podstawy_lvl1[1].pusty_przedzial_1 = 0;
	podstawy_lvl1[1].pusty_przedzial_2 = 170;
	podstawy_lvl1[1].gorna_granica = 310;
	//pietro 2
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 3,
		poziom0 - 210);
	podstawy_lvl1[2].pusty_przedzial_1 = 470;
	podstawy_lvl1[2].pusty_przedzial_2 = 680;
	podstawy_lvl1[2].gorna_granica = 220;
	//pietro 3
	DrawSurface(screen, platforma,
		SCREEN_WIDTH / 3,
		poziom0 - 300);

	podstawy_lvl1[3].gorna_granica = 130;

	//drabina pietro 0-1
	DrawSurface(screen, ladder,
		SCREEN_WIDTH / 3,
		poziom0 - 120 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[0][0] = 200;
	lokalizacja_drabin_lvl1[0][1] = 235;
	lokalizacja_drabin_lvl1[1][0] = 200;
	lokalizacja_drabin_lvl1[1][1] = 235;
	//drabina pietro 1-2
	DrawSurface(screen, ladder,
		3 * SCREEN_WIDTH / 5,
		poziom0 - 210 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[1][2] = 370;
	lokalizacja_drabin_lvl1[1][3] = 400;
	lokalizacja_drabin_lvl1[2][2] = 370;
	lokalizacja_drabin_lvl1[2][3] = 400;
	//drabina pietro 1-3
	DrawSurface(screen, ladder,
		 SCREEN_WIDTH / 2,
		poziom0 - 300 + dlugosc_drabiny);
	lokalizacja_drabin_lvl1[2][4] = 305;
	lokalizacja_drabin_lvl1[2][5] = 335;
	lokalizacja_drabin_lvl1[3][4] = 305;
	lokalizacja_drabin_lvl1[3][5] = 335;

	if (monety[1].czy_nadal_jest)
	{
		DrawSurface(screen, coin,
			SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2,
			SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3);
		monety[1].hitbox_moneta_x_lewo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 - 10;
		monety[1].hitbox_moneta_x_prawo = SCREEN_WIDTH / 2 + SCREEN_HEIGHT / 2 + 10;
		monety[1].hitbox_moneta_y_dolny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 + 10;
		monety[1].hitbox_moneta_y_gorny = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3 - 10;
	}
	DrawSurface(screen, door,
		SCREEN_WIDTH / 3,
		SCREEN_HEIGHT / 2 - 30 - 90);
	drzwi.position_x = SCREEN_WIDTH / 3;
	drzwi.position_y = SCREEN_HEIGHT / 2 - 30 - 90;
	drzwi.hitbox_bottom = SCREEN_HEIGHT / 2 - 30 + 30 - 90;
	drzwi.hitbox_top = SCREEN_HEIGHT / 2 - 30 - 30 - 90;
	drzwi.hitbox_left = SCREEN_WIDTH / 3 - 30;
	drzwi.hitbox_right = SCREEN_WIDTH / 3 + 30;
	//koniec rysowania mapa poziom 1
	//gracz//
	DrawSurface(screen, player,
		SCREEN_WIDTH / 2 + mario.distance * SCREEN_HEIGHT / 3,
		SCREEN_HEIGHT / 2 + mario.distance_y * SCREEN_HEIGHT / 3);

	//beczki//
	DrawSurface(screen, beczka,
		beczki[1].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[1].distance_y_beczki * SCREEN_HEIGHT / 3);
	DrawSurface(screen, beczka,
		beczki[2].distance_beczki * SCREEN_HEIGHT / 3,
		70 + beczki[2].distance_y_beczki * SCREEN_HEIGHT / 3);

	//donkey kong
	DrawSurface(screen, kong,
		SCREEN_HEIGHT / 7,
		45 + SCREEN_HEIGHT / 3);
}
int main_menu(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Renderer* renderer,
SDL_Surface* charset, int jasnozielony, int ciemniejszyzielony, int zielony, int& quit, int flaga_menu, char text[128], char nickname[128], int czarny, int &dlugosc_nickname)
{
	SDL_FillRect(screen, NULL, czarny);

	SDL_Event event;
	DrawRectangle(screen, 4, 160, SCREEN_WIDTH - 8, 120, jasnozielony, zielony);

	sprintf(text, "JEDRZEJ JASINIECKI 197993");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);
	sprintf(text, "WPISZ NAZWE UZYTKOWNIKA LITERY(a-z) Bckspace-usunlitery");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 120, text, charset);
	DrawString(screen, screen->w / 2 - strlen(nickname) * 8 / 2, 140, nickname, charset);


	sprintf(text, "Esc - WYJDZ          SPRAWDZ WYNIKI (NIEDOSTEPNE)");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 180, text, charset);

	sprintf(text, " 1 - POZIOM A         2 - POZIOM B          3 - POZIOM C ");
	DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 230, text, charset);

	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				quit = 1;
				return 0;
			}

			else if (event.key.keysym.sym == SDLK_1)
			{
				return 1;
				break;
			}
			else if (event.key.keysym.sym == SDLK_2)
			{
				return 2;
				break;
			}
			else if (event.key.keysym.sym == SDLK_3)
			{
				return 3;
				break;
			}
			else if (event.key.keysym.sym >= SDLK_a && event.key.keysym.sym <= SDLK_z)
			{
					nickname[dlugosc_nickname] = (char)event.key.keysym.sym;
					nickname[dlugosc_nickname + 1] = '\0'; 
					dlugosc_nickname++;


			}
			// Handle backspace
			else if (event.key.keysym.sym == SDLK_BACKSPACE)
			{
				if (dlugosc_nickname > 0)
				{
					nickname[dlugosc_nickname - 1] = '\0'; // Remove last character
				}
				dlugosc_nickname--;
			}
			break;

		case SDL_QUIT:
			quit = 1;
			return 0;
			
		}
	}

}
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv)
{
	int tick1, tick2, rc, quit,/* [pietro][nr drabiny np drabina pietro 0 (1 lubb 2)] [0][0]  */ lokalizacja_drabin_lvl1[20][20],
		pietra_lvl1[10];
	double deltatime, worldTime;
	SDL_Event event;
	SDL_Surface* screen, * charset, * wall, * ladder, * dirt, * beczka, * serce, * coin, * kong, *door,*podstawa,*platforma,*platforma2,*podstawa2,*queen,*platforma3;
	SDL_Surface* player;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
	struct mario mario;
	struct pietra podstawy_lvl1[10];
	struct beczka beczki[10];
	struct moneta monety[10];
	struct drzwi drzwi;
	//komunikat error
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}
	// tryb pe³noekranowy
	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);

	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_SetWindowTitle(window, "DONKEY KONG - JEDRZEJ JASINIECKI ID_studenta:197993");

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000); 
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);

	// wczytanie obrazków
	charset = SDL_LoadBMP("./cs8x8.bmp");
	if (charset == NULL)
	{
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	SDL_SetColorKey(charset, true, 0x000000);
	player = SDL_LoadBMP("./mario_standing_left.bmp");
	if (player == NULL)
	{
		printf("SDL_LoadBMP(mario_standing_left.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	wall = SDL_LoadBMP("./green_wall.bmp");
	if (wall == NULL)
	{
		printf("SDL_LoadBMP(green_wall.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	ladder = SDL_LoadBMP("./ladder.bmp");
	if (ladder == NULL)
	{
		printf("SDL_LoadBMP(ladder.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	dirt = SDL_LoadBMP("./dirt.bmp");
	if (dirt == NULL)
	{
		printf("SDL_LoadBMP(dirt.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	beczka = SDL_LoadBMP("./beczka1.bmp");
	if (beczka == NULL)
	{
		printf("SDL_LoadBMP(beczka1.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	serce = SDL_LoadBMP("./heart.bmp");
	if (serce == NULL)
	{
		printf("SDL_LoadBMP(heart.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	coin = SDL_LoadBMP("./coin.bmp");
	if (coin == NULL)
	{
		printf("SDL_LoadBMP(coin.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	kong = SDL_LoadBMP("./donkey.bmp");
	if (kong == NULL)
	{
		printf("SDL_LoadBMP(donkey3.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	door= SDL_LoadBMP("./door.bmp");
	if (door == NULL)
	{
		printf("SDL_LoadBMP(door.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	podstawa = SDL_LoadBMP("./yellow.bmp");
	if (podstawa == NULL)
	{
		printf("SDL_LoadBMP(yellow.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	platforma = SDL_LoadBMP("./yellow2.bmp");
	if (platforma == NULL)
	{
		printf("SDL_LoadBMP(yellow2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	podstawa2 = SDL_LoadBMP("./white.bmp");
	if (podstawa2 == NULL)
	{
		printf("SDL_LoadBMP(white.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	platforma2 = SDL_LoadBMP("./white2.bmp");
	if (platforma2 == NULL)
	{
		printf("SDL_LoadBMP(white2.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	platforma3 = SDL_LoadBMP("./white3.bmp");
	if (platforma3 == NULL)
	{
		printf("SDL_LoadBMP(white3.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	queen = SDL_LoadBMP("./queen.bmp");
	if (queen == NULL)
	{
		printf("SDL_LoadBMP(queen.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(charset);
		SDL_FreeSurface(screen);
		SDL_DestroyTexture(scrtex);
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
		return 1;
	}
	//koniec wczytywania

	//kolorki
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
	int zielony = SDL_MapRGB(screen->format, 51, 153, 51);
	int jasnozielony = SDL_MapRGB(screen->format, 0, 255, 0);
	int ciemniejszyzielony = SDL_MapRGB(screen->format, 3, 153, 99);
	int ciemnyczerwoby= SDL_MapRGB(screen->format, 41, 14, 20);
	int jasnoniebieski= SDL_MapRGB(screen->format, 96, 143, 138);
	//koniec kolorków

	tick1 = SDL_GetTicks();
	quit = 0;
	worldTime = 0;
	mario.distance = 0;
	mario.distance_y = 1;
	mario.playerspeed = 0;
	mario.playerspeed_y = 0;
	mario.czy_lokalizacja_drabiny = 0;
	mario.pietro = 0;
	int flaga_menu = 0;
    mario.flaga_czy_strata_hp = 0;
	mario. flaga_czy_w_gore = 0;
	mario. flaga_czy_w_dol = 0;
	mario. czy_w_gore_animacja = 0;
	mario. czy_w_prawo_animacja = 0;
	mario.czy_w_lewo_animacja = 0;
	mario.predkosc_spadku = 1;
	mario.czy_skok = 0;
	mario.predkosc_skoku = 0.0;
	mario.flaga_spadanie = 0;
	mario.spadanie_ktore_pietro = -1;
	mario.czy_jest_na_drabinie = 0;
	mario.czy_double_jump = 0;
	beczki[2].distance_beczki = odleglosc_miedzy_beczkami;
	mario.zycie = 3;
	int pozycja_pomocnicza = 800;
	mario.punkty=0;
	char nickname[128] = "";
	double klatki=0;
	double klatki_gora_dol = 0;
	double zapamietany_distance_mario = 0;
	double zapamietany_distance_y_mario = 1;
	int dlugosc_nickname = 0;


	while (!quit)
	{
		//main menu//
		if (!mario.flaga_czy_strata_hp)
		{
			while (!flaga_menu)
			{
				flaga_menu = main_menu(screen, scrtex, renderer, charset, jasnozielony, ciemniejszyzielony, zielony, quit, flaga_menu, text, nickname, czarny,dlugosc_nickname);
				reset(mario, beczki, worldTime);
				if (quit == 1)
					return 0;

				tick1 = SDL_GetTicks();
			}
			tick2 = SDL_GetTicks();
			deltatime = (tick2 - tick1) * 0.001;
			tick1 = tick2;
			//klatki do animacji
			klatki += deltatime;
			if (klatki >= 0.5)
				klatki -= 0.5; 

			worldTime += deltatime;
			mario.mario_position_x = SCREEN_WIDTH / 2 + mario.distance * SCREEN_HEIGHT / 3;
			mario. mario_position_y = SCREEN_HEIGHT / 2 + mario.distance_y * SCREEN_HEIGHT / 3;
			mario.distance += mario.playerspeed * deltatime;
			mario.distance_y += mario.playerspeed_y * deltatime;

			animacje(klatki, mario, player, beczka, kong, beczki,screen,charset,text,flaga_menu);
			logika_beczki(deltatime, beczki, 2, podstawy_lvl1);
			ograniczanie_mapy(mario, blokowoanie_z_lewej_dla_distance, blokowoanie_z_prawej_dla_distance);
			logika_drabiny(mario, lokalizacja_drabin_lvl1, podstawy_lvl1);
			skakanie_na_wyzsza_platforme(mario, podstawy_lvl1);
			spadanie(mario, podstawy_lvl1, deltatime);
			skakanie(mario, deltatime);
			sprawdzanie_kolizji_mario_z_beczka(mario, beczki);
			porazka(flaga_menu, mario);
			
			//mapa poziom 1
			if (flaga_menu == 1)
			mapa_level_pierwszy(screen, dirt, wall, ladder, coin, player, beczka, kong, mario, podstawy_lvl1, beczki, monety, lokalizacja_drabin_lvl1,czarny,door,drzwi);
			if (flaga_menu == 2)
			{
				mapa_level_drugi(screen, podstawa, platforma, ladder, coin, player, beczka, kong, mario, podstawy_lvl1, beczki, monety, lokalizacja_drabin_lvl1, ciemnyczerwoby, door, drzwi);
			}
			if (flaga_menu == 3)
			{
				mapa_level_trzeci(screen, podstawa2, platforma2, ladder, coin, player, beczka, kong, mario, podstawy_lvl1, beczki, monety, lokalizacja_drabin_lvl1, jasnoniebieski, queen, drzwi, platforma3);
			}
			//sprawdzanie na ktorym pietrze jest gracz
			sprawdzanie_na_ktorym_pietrze_jest_gracz(mario, podstawy_lvl1);

			//sprawdzanie na którym piêtrze jest beczka
			for (int i = 1; i <= 2; i++)
			{
				if (beczki[i].beczka_position_y <= 230)
					beczki[i].pietro_beczki = 2;
				else if (beczki[i].beczka_position_y <= 321)
					beczki[i].pietro_beczki = 1;
				else if (beczki[i].beczka_position_y <= 410)
					beczki[i].pietro_beczki = 0;

			}
			sprawdzanie_czy_moze_wejsc_na_drabine(mario, lokalizacja_drabin_lvl1);
			

			
			punkty_za_skok_nad_beczka(mario, beczki, screen, charset, text);
			punkty_za_monete_i_przejscie_poziomu(mario, monety, drzwi, pozycja_pomocnicza, flaga_menu, screen, charset, text,beczki,worldTime);
			wyswietlanie_zycia(screen, serce, mario);

			//wyswietlanie punktow
			sprintf(text,"%d", mario.punkty);
			DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT / 7, text, charset);

			menu_gry(screen, charset, czerwony, niebieski, worldTime, text);
			kontynuacja_po_stracie_zycia(screen, charset, niebieski, czerwony, mario, flaga_menu, text, monety);
			koniec_zycia(screen, charset, niebieski, czerwony, mario, flaga_menu, text, monety);


			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

		}
		// obs³uga zdarzeñ
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = 1;
				else if (event.key.keysym.sym == SDLK_n)
				{
					mario.distance = 0;
					mario.distance_y = 1;
					worldTime = 0;
				}
				else if (event.key.keysym.sym == SDLK_LEFT)
				{
					mario.czy_w_lewo_animacja = 1;
					if (flaga_menu == 3)
						mario.playerspeed = (-1) * predkosc_gracza / 3;
					else
					mario.playerspeed = (-1) * predkosc_gracza;
					mario.flaga_czy_w_gore = 0;
					mario.flaga_czy_w_dol = 0;
				}
				else if (event.key.keysym.sym == SDLK_RIGHT)
				{
					mario.czy_w_prawo_animacja = 1;
					if (flaga_menu == 3)
						mario.playerspeed =  predkosc_gracza / 3;
					else
						mario.playerspeed =  predkosc_gracza;
					mario.flaga_czy_w_gore = 0;
					mario.flaga_czy_w_dol = 0;
				}
				else if (event.key.keysym.sym == SDLK_UP)
				{
					mario.czy_w_gore_animacja = 1;
					if (mario.czy_lokalizacja_drabiny)
					{
						mario.playerspeed = 0;
						mario.playerspeed_y = (-1) * predkosc_gracza;
						mario.flaga_czy_w_gore = 1;
						mario.flaga_czy_w_dol = 0;
					}
				}
				else if (event.key.keysym.sym == SDLK_DOWN)
				{
					mario.czy_w_gore_animacja = 1;
					if (mario.czy_lokalizacja_drabiny)
					{
						mario.playerspeed_y = predkosc_gracza;
						mario.playerspeed = 0;
						mario.flaga_czy_w_gore = 0;
						mario.flaga_czy_w_dol = 1;
					}
				}
				else if (event.key.keysym.sym == SDLK_SPACE && mario.czy_skok != 1)
				{
					if(mario.playerspeed>0)
						player = SDL_LoadBMP("./skok_right.bmp");
					if (mario.playerspeed <= 0)
						player = SDL_LoadBMP("./skok_left.bmp");
					mario.czy_skok = 1;
					if (!mario.czy_double_jump)
						mario.predkosc_skoku = (-1) * wysokosc_skoku;
					else
						mario.predkosc_skoku = (-1) * wysokosc_double_jump;
				}
				else if (event.key.keysym.sym == SDLK_k)
				{
					mario.flaga_czy_strata_hp = 0;
					mario.czy_skok = 0;
					mario.flaga_spadanie = 0;
					mario.distance = 0;
					mario.distance_y = 1;
					
					for (int i = 1; i <= 2; i++)
					{
						if (i == 1)
						{
							beczki[i].distance_beczki = 0;
							beczki[i].distance_y_beczki = 1;
						}
						if (i == 2)
						{
							beczki[i].distance_beczki = odleglosc_miedzy_beczkami;
							beczki[i].distance_y_beczki = 1;
						}
						beczki[i].flaga_spadanie_beczki = 0;
						if (beczki[i].pietro_beczki == 1)
							beczki[i].pietro_beczki = 2;
					}
					tick1 = SDL_GetTicks();
				}
				else if (event.key.keysym.sym == SDLK_p)
				{
					mario.flaga_czy_strata_hp = 0;
					mario.distance = 0;
					mario.distance_y = 1;
					mario.czy_skok = 0;
					mario.flaga_spadanie = 0;
					for (int i = 1; i <= 2; i++)
					{
						if (i == 1)
						{
							beczki[i].distance_beczki = 0;
							beczki[i].distance_y_beczki = 1;
						}
						if (i == 2)
						{
							beczki[i].distance_beczki = odleglosc_miedzy_beczkami;
							beczki[i].distance_y_beczki = 1;
						}
						beczki[i].flaga_spadanie_beczki = 0;
						if (beczki[i].pietro_beczki == 1)
							beczki[i].pietro_beczki = 2;
					}
					tick1 = SDL_GetTicks();
				}
				break;

			case SDL_KEYUP:
				mario.czy_w_gore_animacja = 0;
				mario.czy_w_prawo_animacja = 0;
				mario.czy_w_lewo_animacja = 0;
				player = SDL_LoadBMP("./mario_standing_left.bmp");
				mario.playerspeed_y = 0;
				mario.playerspeed = 0;
				break;
			case SDL_QUIT:
				quit = 1;
				break;
			}
		}

	}
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(player);
	SDL_FreeSurface(wall);
	SDL_FreeSurface(ladder);
	SDL_FreeSurface(dirt);
	SDL_FreeSurface(beczka);
	SDL_FreeSurface(kong);
	SDL_FreeSurface(serce);
	SDL_FreeSurface(coin);
	SDL_FreeSurface(door);
	SDL_FreeSurface(platforma);
	SDL_FreeSurface(podstawa);
	SDL_FreeSurface(platforma2);
	SDL_FreeSurface(platforma3);
	SDL_FreeSurface(podstawa2);
	SDL_FreeSurface(queen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;

};
