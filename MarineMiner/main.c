

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <SDL2/SDL_ttf.h>
TTF_Font* font;

void SayiyiEkranaYaz(SDL_Renderer* renderer, TTF_Font* font, float alan, int * point,int elemanSay,int kareGenislik,int kareUzunluk,int satirSay, SDL_Color renk) {
    char alanStr[64];
    snprintf(alanStr, sizeof(alanStr), "%.2f", alan);
    SDL_Surface* yaziYuzey = TTF_RenderText_Solid(font, alanStr, renk);
    SDL_Texture* yaziTexture = SDL_CreateTextureFromSurface(renderer, yaziYuzey);
    int x = 0, y = 0 ;
    for (int i = 0; i<elemanSay;i+=2){
        x += point[i];
        y += point[i+1];
    }
    x = (x / (elemanSay/2)-1)*kareGenislik;
    y = (y / (elemanSay/2))*kareUzunluk;

    SDL_Rect metinRect;
    metinRect.x = x;
    metinRect.y = y;
    metinRect.w = yaziYuzey->w;
    metinRect.h = yaziYuzey->h;

    SDL_RenderCopy(renderer, yaziTexture, NULL, &metinRect);

    SDL_DestroyTexture(yaziTexture);
    SDL_FreeSurface(yaziYuzey);
}

//seklin kenarlarini 10x10'luk karelere boler.
void cizimYapma(SDL_Renderer* renderer, int* point, int elemanSay){
    int kareBoyut = 10;
    int sutunSay = 60;
    int satirSay = 40;
    int kareGenislik = 1200 / sutunSay;
    int kareUzunluk = 800 / satirSay;
         for (int i = 0; i < elemanSay; i += 2) {
            int x1 = point[i] * kareGenislik;
            int y1 = (point[i + 1]) * kareUzunluk;
            int x2 = point[(i + 2) % elemanSay] * kareGenislik;
            int y2 = (point[(i + 3) % elemanSay]) * kareUzunluk;

            int minX = x1 < x2 ? x1 : x2;
            int minY = y1 < y2 ? y1 : y2;
            int maxX = x1 > x2 ? x1 : x2;
            int maxY = y1 > y2 ? y1 : y2;

            float deltaX = x2 - x1;
            float deltaY = y2 - y1;
            float egim = deltaY / deltaX;

            for (int y = minY; y <= maxY; y += kareBoyut) {
                int x = x1 + (y - y1) / egim;
                int kareX = x - kareBoyut / 2;
                int kareY = y - kareBoyut / 2;

                SDL_Rect kare = {kareX, kareY, kareBoyut, kareBoyut};
                SDL_RenderDrawRect(renderer, &kare);
            }
                }
}

float alanHesaplama (int *dizi, int diziBoyut){
    float alan = 0 ;
    int tmp = 0, tmp2 = 0;
    int carpilanDeger1 = 0, carpilanDeger2 = 0;
    for (int i = 0; i < diziBoyut-1 ; i+=2){
        if ( i == diziBoyut-2){
            tmp = dizi[i]*dizi[1];
        }
       else{
            tmp = dizi[i] * dizi[i+3];
       }
        carpilanDeger1+=tmp;
    }

    for (int i = 1 ; i < diziBoyut ; i+=2){
        if (i == diziBoyut-1){
            tmp2 = dizi[i] * dizi[0];
        }
        else {
            tmp2 = dizi[i]*dizi[i+1];
        }
        carpilanDeger2 += tmp2;
    }

    alan = 0.5 * fabs(carpilanDeger1 - carpilanDeger2);
    //printf("alan = %f",alan);

    return alan;
}

// "scanline fill" algoritmasi ile çokgenin içini dolduran islev
void scanlineFill(SDL_Renderer* renderer, int* points, int pointCount, int y, int kareGenislik, int kareUzunluk) {
    for (int x = 0; x < 1200; x++) {
        int intersections = 0;
        for (int i = 0; i < pointCount; i += 2) {
            int x1 = points[i] * kareGenislik;
            int y1 = (points[i + 1]-1) * kareUzunluk+kareUzunluk;
            int x2 = points[(i + 2) % pointCount] * kareGenislik;
            int y2 = (points[(i + 3) % pointCount]-1) * kareUzunluk+kareUzunluk;
            if ((y1 <= y && y2 > y) || (y1 > y && y2 <= y)) {
                //(y - y1) / (y2 - y1) = (x - x1) / (x2 - x1) faydanilan denklem
                float t = (float)(y - y1) / (y2 - y1);
                float xi = x1 + t * (x2 - x1);
                if (xi < x) {
                    intersections++;
                }
            }
        }
        if (intersections % 2 == 1) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}
void drawPolygon(SDL_Renderer* renderer, int* points, int pointCount, int kareGenislik, int kareUzunluk) {
    for (int a = 0; a < pointCount - 2; a += 2) {
        int x1 = points[a] * kareGenislik;
        int y1 = (points[a + 1]-1) * kareUzunluk + kareUzunluk;
        int x2 = points[a + 2] * kareGenislik;
        int y2 = (points[a + 3]-1) * kareUzunluk + kareUzunluk;
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }

    int x1 = points[pointCount - 2] * kareGenislik;
    int y1 = (points[pointCount - 1]-1) * kareUzunluk + kareUzunluk;
    int x2 = points[0] * kareGenislik;
    int y2 = (points[1]-1) * kareUzunluk + kareUzunluk;
    SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
}

char received_data[4096]; // Maksimum veri boyutu
size_t data_size = 0; // Alinan verinin boyutu

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;

    if (data_size + realsize <= sizeof(received_data)) {
        memcpy(received_data + data_size, contents, realsize);
        data_size += realsize;
    } else {
        fprintf(stderr, "Alinan veri boyutu mevcut dizinin kapasitesini asti.\n");
    }

    return realsize;
}

int main(int argc, char *argv[]) {
    CURL *curl;
    CURLcode kontrol;
    curl = curl_easy_init();
    if (curl) {
        char url[256];
        printf("Verileri almak istediginiz URL'yi girin:\n");
        scanf("%s", url);

        // CURL ayarlari
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        kontrol = curl_easy_perform(curl);

        if (kontrol == CURLE_OK) {
            printf("Veri alindi. Toplam alinan veri boyutu: %u\n", data_size);

            // received_data dizisinin içerigini kontrol et
            printf("received_data dizisi:\n%s\n", received_data);
        } else {
            fprintf(stderr, "CURL hata: %s\n", curl_easy_strerror(kontrol));
        }
    }
    char dizi [4096];
    strcpy(dizi,received_data);
    int diziBoyutu = sizeof (dizi) / sizeof (dizi[0]);
    int secilensay = 0, sondajMaliyet=0, platformMaliyet=0;
    char ctrldizi[4]="",kesDizi[4]=""; // Ekstra bir karakterlik alan '\0' (null karakter) için
    printf("Kullanmak istediginiz satiri seciniz: ");
    scanf("%d", &secilensay);
    printf("Birim sondaj maliyetini giriniz: ");
    scanf("%d", &sondajMaliyet);
    printf("Birim platform maliyetini giriniz: ");
    scanf("%d", &platformMaliyet);
    sprintf(ctrldizi, "%dB", secilensay);
    int secilensay1 =secilensay+1;
    sprintf(kesDizi,"%dB",secilensay1);
    int indeks = 0,indeks1=0;
    char *bulunanIndex = strstr(dizi, ctrldizi);
    char *bulunanIndex1 = strstr(dizi,kesDizi);
    indeks1 = bulunanIndex1 - dizi - 2;
    char kullanilacakDizi[80] = "";
    int f = 0;

    if (bulunanIndex != NULL && bulunanIndex1 != NULL && secilensay<10) {
        indeks = bulunanIndex - dizi + 2;
        //printf("%d indeks\n", indeks);
        }

    if (bulunanIndex != NULL && bulunanIndex1 != NULL && secilensay>=10) {
        indeks = bulunanIndex - dizi + 3;
        //printf("%d indeks\n", indeks);
        }

    for (int i = indeks; i<=indeks1; i++ ){
        kullanilacakDizi[f] = dizi[i];
        f++;
    }

    if (bulunanIndex1 == NULL){
        if(secilensay<10){
            indeks = bulunanIndex - dizi+2;
            }
        else if (secilensay>=10){
            indeks = bulunanIndex - dizi +3;
        }
       for (int i = indeks; i<= diziBoyutu-1; i++ ){
        kullanilacakDizi[f] = dizi[i];
        f++;
        }
    }

        const char *ayrac = "(,)";
        int point[80];
        char *tmp =strtok(kullanilacakDizi,ayrac);
        int i = 0;
        while (tmp != NULL) {
            point[i] = atoi(tmp);
            tmp = strtok(NULL, ayrac);
            i++;
        }
        int elemanSay = i;
        // Elde edilen diziyi yazdirma
        /*for (int j = 0; j < i-1; j++) {
            printf("%d ", point[j]);
        }*/

    int point1[100] = {0}, point2[100] = {0}, point3[100] = {0};
    float point1_alan = 0, point2_alan = 0, point3_alan = 0;
    int found = 0;
    //int elemanSay = sizeof(point) / sizeof(point[0]);
    int x=0, y=0, z=0, a=0;
    int point1_eleman_sayisi = 0, point2_eleman_sayisi = 0 , point3_eleman_sayisi=0;

    for (x = 2; x < elemanSay; x += 2) {
        if (point[0] == point[x] && point[1] == point[x + 1]) {
            found = 1;
            break;
        }
    }
    if(found){
    for (a = 0; a < x; a++) {
        point1[a] = point[a];
    }}
        printf("point1: ");
        for (int i = 0; point1[i] != 0; i++) {
            printf("%d ", point1[i]);
        }
        if (point1[0] != 0) {
        while (point1[point1_eleman_sayisi] != 0) {
            point1_eleman_sayisi++;
        }
        printf("point1 eleman sayisi: %d\n", point1_eleman_sayisi);
    } else {
        printf("point1 dizisi bos.\n");
    }
    point1_alan = alanHesaplama(point1 , point1_eleman_sayisi);
    printf("Alan1 = %f",point1_alan);
    if (point[x + 3] != 0) {
        for (y = x+4 ; y < elemanSay; y += 2) {
            if (point[x + 2] == point[y] && point[x + 3] == point[y + 1]) {
                found = 2;
                break;
            }
        }
        if (found ==2){
        for (a = 0; a < y-x-2 ; a++) {
            point2[a] = point[x+a+2];
        }
    } }
        if (point2[0] != 0) {
        printf("\npoint2: ");
        for (int i = 0; i < 100 && point2[i] != 0; i += 2) {
            printf("%d %d ", point2[i], point2[i + 1]);
        }}

     if (point2[0] != 0) {
            while (point2[point2_eleman_sayisi] != 0) {
                point2_eleman_sayisi++;
            }
            printf("\npoint2 eleman sayisi: %d\n", point2_eleman_sayisi);
            point2_alan = alanHesaplama(point2 , point2_eleman_sayisi);
            printf("Alan2 = %f",point2_alan);

        } else {
            printf("\npoint2 dizisi bos.\n");
        }


    if (point[x + 3] != 0){
    if (point[y + 3] != 0) {
        for (z = y + 4; z < elemanSay; z += 2) {
            if (point[y + 2] == point[z] && point[y + 3] == point[z + 1]) {
                    found =3;
                break;
            }
        }
        if (found==3){
        for (a = 0; a < z - y - 2; a++) {
            point3[a] = point[y + a + 2];
        }
         if (point3[0] != 0) {
                while (point3[point3_eleman_sayisi] != 0) {
                    point3_eleman_sayisi++;
                }
                printf("\npoint3 eleman sayisi: %d\n", point3_eleman_sayisi);
                if (point[y + 2] != 0){
                printf("point3: ");
        for (int i = 0; i < 100 && point3[i] != 0; i += 2) {
            printf("%d %d ", point3[i], point3[i + 1]);}
            point3_alan = alanHesaplama(point3 , point3_eleman_sayisi);
            printf("Alan3 = %f",point3_alan);
            }
            }} else {
                printf("\npoint3 dizisi bos.\n");
            }


    }}
    if (point2_alan != 0 && point3_alan != 0 ){
        float toplamAlan = point1_alan + point2_alan + point3_alan;
        printf("\nToplam alan = %f",toplamAlan);
    }
    else if (point2_alan != 0 && point3_alan == 0 ){
        float toplamAlan = point1_alan + point2_alan + point3_alan;
        printf("\nToplam alan = %f",toplamAlan);
    }


    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL baslatma hatasi: %s\n", SDL_GetError());
        return 1;
    }
    if (TTF_Init() < 0) {
        printf("TTF baþlatma hatasý: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }


    SDL_Window *window = SDL_CreateWindow("Koordinat Sistemi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer) {
        SDL_Log("Window or Renderer creation failed: %s\n", SDL_GetError());
        return 2;
    }

    SDL_Event event;
    int quit = 0;

    int sutunSay = 60;
    int satirSay = 40;
    int kareGenislik = 1200 / sutunSay;
    int kareUzunluk = 800 / satirSay;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = 1;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        TTF_Font* font = TTF_OpenFont("fonts/arial.ttf", 24);
        if (font == NULL) {
        printf("Yazý tipi yüklenemedi: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

        SDL_SetRenderDrawColor(renderer, 176, 224, 230, 255);

        for (int i = 0; i < sutunSay; i++) {
            for (int j = 0; j < satirSay; j++) {
                int x = i * kareGenislik;
                int y = (j) * kareUzunluk;

                SDL_RenderDrawLine(renderer, x, y, x + kareGenislik, y);
                SDL_RenderDrawLine(renderer, x + kareGenislik, y, x + kareGenislik, y + kareUzunluk);
                SDL_RenderDrawLine(renderer, x + kareGenislik, y + kareUzunluk, x, y + kareUzunluk);
                SDL_RenderDrawLine(renderer, x, y + kareUzunluk, x, y);
            }
        }
         SDL_SetRenderDrawColor(renderer, 139, 101, 139, 255);

        //Çokgen çizimi
        drawPolygon(renderer, point1 , point1_eleman_sayisi, kareGenislik, kareUzunluk);
        for (int y = 0; y < 800; y++) {
            scanlineFill(renderer, point1, point1_eleman_sayisi, y, kareGenislik, kareUzunluk);
        }
        if (point2[0]!=0){
        drawPolygon(renderer, point2 , point2_eleman_sayisi, kareGenislik, kareUzunluk);
        for (int y = 0; y < 800; y++) {
            scanlineFill(renderer, point2, point2_eleman_sayisi, y, kareGenislik, kareUzunluk);
        }}
        if (point3[0]!=0){
        drawPolygon(renderer, point3 , point3_eleman_sayisi, kareGenislik, kareUzunluk);
        for (int y = 0; y < 800; y++) {
            scanlineFill(renderer, point3, point3_eleman_sayisi, y, kareGenislik, kareUzunluk);
        }}

        SDL_Color renk = {255, 255, 255};
        SayiyiEkranaYaz(renderer, font, point1_alan , point1, point1_eleman_sayisi,kareGenislik,kareUzunluk,satirSay, renk);
        if(point2[0]!=0)
            SayiyiEkranaYaz(renderer, font, point2_alan , point2, point2_eleman_sayisi,kareGenislik,kareUzunluk,satirSay, renk);
        if(point3[0]!=0)
            SayiyiEkranaYaz(renderer, font, point3_alan , point3, point3_eleman_sayisi,kareGenislik,kareUzunluk,satirSay, renk);


        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);


    SDL_Window *window1 = SDL_CreateWindow("Koordinat Sistemi", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, 0);
    SDL_Renderer *renderer1 = SDL_CreateRenderer(window1, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!window || !renderer) {
        SDL_Log("Window or Renderer creation failed: %s\n", SDL_GetError());
        return 2;
    }
    SDL_Event event1;
    int quit1 = 0;

    while (!quit1) {
        while (SDL_PollEvent(&event1)) {
            if (event1.type == SDL_QUIT) {
                quit1 = 1;
            }
        }
        SDL_SetRenderDrawColor(renderer1, 0, 0, 0, 255);
        SDL_RenderClear(renderer1);

        SDL_SetRenderDrawColor(renderer1, 147, 112, 219, 255);
            for (int i = 0; i < 60; i++) {
            for (int j = 0; j < 40; j++) {
                int x = i * kareGenislik;
                int y = j * kareUzunluk;

                SDL_RenderDrawLine(renderer1, x, y, x + kareGenislik, y);
                SDL_RenderDrawLine(renderer1, x + kareGenislik, y, x + kareGenislik, y + kareUzunluk);
                SDL_RenderDrawLine(renderer1, x + kareGenislik, y + kareUzunluk, x, y + kareUzunluk);
                SDL_RenderDrawLine(renderer1, x, y + kareUzunluk, x, y);
            }
        }
        SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255);
        drawPolygon(renderer1, point1 , point1_eleman_sayisi, kareGenislik, kareUzunluk);
        SDL_SetRenderDrawColor(renderer1, 255, 192, 0, 255);
        cizimYapma(renderer1,point1,point1_eleman_sayisi);

        if (point2[0]!=0){
        SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255);
        drawPolygon(renderer1, point2 , point2_eleman_sayisi, kareGenislik, kareUzunluk);
        SDL_SetRenderDrawColor(renderer1, 255, 192, 0, 255);
        cizimYapma(renderer1,point2,point2_eleman_sayisi);
        }
        if (point3[0]!=0){
        SDL_SetRenderDrawColor(renderer1, 255, 255, 255, 255);
        drawPolygon(renderer1, point3 , point3_eleman_sayisi, kareGenislik, kareUzunluk);
        SDL_SetRenderDrawColor(renderer1, 255, 192, 0, 255);
        cizimYapma(renderer1,point3,point3_eleman_sayisi);
        }
         SDL_RenderPresent(renderer1);

        }


    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

