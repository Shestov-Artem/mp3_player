#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <iostream>
#include <filesystem>
#include <string.h>
#include <sstream>
#include <cmath>

namespace fs = std::filesystem;

#pragma warning(disable : 4996)

SDL_Renderer* renderer = nullptr;

SDL_Texture* LoadImage(std::string file) {
    SDL_Surface* loadedImage = nullptr;
    SDL_Texture* texture = nullptr;
    loadedImage = IMG_Load(file.c_str());
    if (loadedImage != nullptr) {
        texture = SDL_CreateTextureFromSurface(renderer, loadedImage);
        SDL_FreeSurface(loadedImage);
    }
    else
        std::cout << SDL_GetError() << std::endl;
    return texture;
}

//Converting a WChar string to a Ansi string
char* w2c(char* pcstr, const wchar_t* pwstr, size_t len)
{
    int nlength = wcslen(pwstr);
    //Gets the converted length
    int nbytes = WideCharToMultiByte(0, 0, pwstr, nlength, NULL, 0, NULL, NULL);
    if (nbytes > len)   nbytes = len;
    //With the results obtained above, convert unicode characters to ASCII characters
    WideCharToMultiByte(0, 0, pwstr, nlength, pcstr, nbytes, NULL, NULL);
    return pcstr;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();
    setlocale(LC_ALL, "RUS");

    SDL_Window* window = SDL_CreateWindow("ARTEMOPLEER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 500, 0);                       //������ ���� �� ��������
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //�������� ���� � �����, ��� ��������� ����������� ����
    wchar_t path[MAX_PATH];
    GetCurrentDirectory(sizeof(path), path);
    //wprintf(L"%s\n", path);

    //����������� path ���� wchar_t � ��� char*, � ����� � string
    char* pcstr = (char*)malloc(sizeof(char) * (2 * wcslen(path) + 1));
    memset(pcstr, 0, 2 * wcslen(path) + 1);
    w2c(pcstr, path, 2 * wcslen(path) + 1);
    std::string a = pcstr;
    std::cout << "���� � ����� � �����������: " << a << std::endl;
    free(pcstr);

    //������� ���������� ������ mp3 � ����� �����
    const std::string folderPath = a;
    int  KolMusic = 0;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".mp3") {
            KolMusic++;
        }
    }
    std::cout << "������� " << KolMusic << " ����� mp3:" << std::endl;

    //music
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: ");
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: ");
        return 1;
    }

    //������� ������ �� �������� ����� �� ����� �����
    char** c = new char* [KolMusic];
    for (int i = 0; i < KolMusic; i++)
        c[i] = new char[100];
    int i = 0;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".mp3") {
            strcpy(c[i], entry.path().filename().string().c_str());
            std::cout << c[i] << std::endl;
            i++;
        }
    }

    //������ ���������� 
    SDL_Texture* BackG = LoadImage("BackGroundPleer.png"); //������ ���
    SDL_Texture* StopG = LoadImage("PlayStop.png");
    SDL_Texture* PrevG = LoadImage("Prev.png");
    SDL_Texture* NextG = LoadImage("Next.png");
    SDL_Texture* RewindFG = LoadImage("RewindForward.png");
    SDL_Texture* RewindBG = LoadImage("RewindBack.png");
    SDL_Texture* OnOff = LoadImage("OnOff.png");

    SDL_Surface* surface = IMG_Load("VolCircle.png");      //������ ���������� ����������
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    // ����������� �������������� ��� ��������� ����������� (������ ���� �������� �������� ������� ����� �����)
    SDL_Rect srcRect = { 0, 0, 500, 500 };
    // ����������� �������������� ��� �������� ����������� (����� ��� ���������� � ������)
    SDL_Rect dstRect = { 150, 195, 110, 110 };
    double angle = 45;    // ���� �������� � ��������

    //������� �� ����� �������� �����
    TTF_Font* font = TTF_OpenFont("Machine.ttf", 17);
    if (!font) {
        std::cout << "���������� ���������� �������� �����";
        return 1;
    }
    SDL_Color textColor = { 84, 255, 159 }; // White color: 255,255,255

    bool music = true; //����������� � ����� ����� 1 ���
    int NowMusic = 0;  //����� �������� �����
    bool Stop = true;  //�����
    int Volume = 32;   //��������� ������, �������� 128
    double Position = 0;  //������� ����� ��������� ����� 

    bool VolmGb = false;  //������ ��� ������� ������ ���������
    bool VolpGb = false;
    bool NextGb = false;
    bool PrevGb = false;
    bool StopGb = false;

    std::string VolProc;  //����� ��� ������ ���������
    double VolProcD;      //��������� � ���������

    std::string MusText;  //�������� �����

    bool NextButton = false;  //true ���� ������ ���������� �� ������ ������
    bool PrevButton = false;
    bool StopButton = false;

    bool Rewind10secButton = false;  //�������� �������
    bool Rewind10sec = false;        //true ���� ������ ���������� �� ������ ������
    bool RewindBackButton = false;
    bool RewindBack = false;

    double LenMus; //����� ������ � ��������
    int LenMusMin; //����� ������ � ������ �������
    int LenMusSec; //���������� �������

    bool On = true;
    bool OnGb = false;
    bool OnButton = false;

    int close = 0;
    while (!close) {// animation loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {// ���������� ���������
            VolmGb = false;
            VolpGb = false;
            NextGb = false;
            PrevGb = false;
            StopGb = false;
            Rewind10secButton = false;
            RewindBackButton = false;
            OnGb = false;

            switch (event.type) {
            case SDL_QUIT:
                // ���������� ������� ��������
                close = 1;
                break;
            case SDL_KEYDOWN:
                // API ���������� ��� ����������� ������� �������
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_LEFT:
                    NowMusic--;  //������� �����
                    music = true; //������ ��� ���� ����������� ��������� ���� 1 ���
                    Stop = true;  //����� ���������� ���� ���� ���� ����������� ���
                    PrevGb = true; //�������� ������� ������
                    Position = 0;  //�������� ���������
                    break;
                case SDL_SCANCODE_RIGHT:
                    NowMusic++;
                    music = true;
                    Stop = true;
                    NextGb = true;
                    Position = 0;
                    break;
                case SDL_SCANCODE_SPACE:
                    if (Stop) {
                        Mix_PauseMusic();
                        Stop = false;
                        StopGb = true;
                    }
                    else {
                        Mix_ResumeMusic();
                        Stop = true;
                        StopGb = true;
                    }
                    break;
                case SDL_SCANCODE_UP:
                    if (Volume < 128) Volume++;
                    VolpGb = true;
                    break;
                case SDL_SCANCODE_DOWN:
                    if (Volume > 0) Volume--;
                    VolmGb = true;
                    break;
                case SDL_SCANCODE_Q:
                    Mix_RewindMusic();   //������������ ����� � ������
                    RewindBackButton = true;
                    Position = 0;
                    break;
                case SDL_SCANCODE_W:
                    Position = Position + 10.0;
                    Mix_SetMusicPosition(Position);   //��������� ������ �� 10 ������
                    Rewind10secButton = true;
                    break;
                case SDL_SCANCODE_BACKSPACE:
                    if (On) {
                        Mix_PauseMusic();
                        On = false;
                        OnGb = true;
                    }
                    else {
                        Mix_ResumeMusic();
                        On = true;
                        OnGb = true;
                    }
                    break;
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                // API ���������� ��� ����������� ������� ������ ����
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    if (NextButton) {
                        NowMusic++;
                        music = true;  //��������� ������ 1 ���
                        Stop = true;   //����� ���� ������� ��� ������������ �����
                        NextGb = true;  //������� ������� ������
                        Position = 0;  //�������� ���������
                    }
                    if (PrevButton) {
                        NowMusic--;
                        music = true;
                        Stop = true;
                        PrevGb = true;
                        Position = 0;
                    }
                    if (StopButton) {
                        if (Stop) {
                            Mix_PauseMusic();
                            Stop = false;
                            StopGb = true;
                        }
                        else {
                            Mix_ResumeMusic();
                            Stop = true;
                            StopGb = true;
                        }
                    }
                    if (RewindBack) {
                        Mix_RewindMusic();
                        RewindBackButton = true;
                        Position = 0;
                    }
                    if (Rewind10sec) {
                        Position = Position + 10.0;
                        Mix_SetMusicPosition(Position);   //��������� ������ �� 10 ������
                        Rewind10secButton = true;
                    }
                    if (OnButton) {
                        if (On) {
                            Mix_PauseMusic();
                            On = false;
                            OnGb = true;
                        }
                        else {
                            Mix_ResumeMusic();
                            On = true;
                            OnGb = true;
                        }
                    }
                    break;
                }
            }
            //���������� ����
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, BackG, NULL, NULL);

            //���������� ������� ������
            //������� � ������������� �����
            if (mouseX > 50 && mouseX < 112 && mouseY > 235 && mouseY < 255) {        //Prew
                PrevButton = true;
                NextButton = false;
                StopButton = false;
                if (PrevGb) SDL_RenderCopy(renderer, PrevG, NULL, NULL);
            }
            else if (mouseX > 300 && mouseX < 350 && mouseY > 235 && mouseY < 255) {  //Next
                PrevButton = false;
                NextButton = true;
                StopButton = false;
                if (NextGb) SDL_RenderCopy(renderer, NextG, NULL, NULL);
            }
            else if (mouseX > 150 && mouseX < 255 && mouseY > 200 && mouseY < 300) {  //Stop/Play
                PrevButton = false;
                NextButton = false;
                StopButton = true;
                if (StopGb) SDL_RenderCopy(renderer, StopG, NULL, NULL);
            }
            else {
                PrevButton = false;
                NextButton = false;
                StopButton = false;
            }
            //���������
            if (mouseX > 290 && mouseX < 360 && mouseY > 170 && mouseY < 200) {  //��������� �� 10 ��� ������
                Rewind10sec = true;
                if (Rewind10secButton) SDL_RenderCopy(renderer, RewindFG, NULL, NULL);
            }
            else Rewind10sec = false;
            if (mouseX > 290 && mouseX < 360 && mouseY > 300 && mouseY < 325) {  //��������� � ������ �����
                RewindBack = true;
                if (RewindBackButton) SDL_RenderCopy(renderer, RewindBG, NULL, NULL);
            }
            else RewindBack = false;
            if (mouseX > 873 && mouseX < 932 && mouseY > 125 && mouseY < 150) {  //���/���� �����
                OnButton = true;
                if (OnGb) SDL_RenderCopy(renderer, OnOff, NULL, NULL);
            }
            else OnButton = false;

            //�������� ������� ������ � ����������
            if (RewindBackButton) SDL_RenderCopy(renderer, RewindBG, NULL, NULL);
            if (Rewind10secButton) SDL_RenderCopy(renderer, RewindFG, NULL, NULL);
            if (StopGb) SDL_RenderCopy(renderer, StopG, NULL, NULL);
            if (NextGb) SDL_RenderCopy(renderer, NextG, NULL, NULL);
            if (PrevGb) SDL_RenderCopy(renderer, PrevG, NULL, NULL);
            if (OnGb) SDL_RenderCopy(renderer, OnOff, NULL, NULL);

            //���������� �������
            if (VolmGb && angle > 0) angle--;
            if (VolpGb && angle < 128) angle++;
            SDL_RenderCopyEx(renderer, texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE);

            if (StopGb) SDL_RenderCopy(renderer, StopG, NULL, NULL);   //�������� ������� �������� Play Stop

            //������� �������� ������ ������������ ����� 
            if (NowMusic >= KolMusic)
                NowMusic = 0;
            if (NowMusic < 0)
                NowMusic = KolMusic - 1;

            if (On) { //���� ����� �������
                Mix_Music* Fmusic = Mix_LoadMUS(c[NowMusic]);
                if (!Fmusic) {
                    SDL_Log("Unable to load music file: %s", Mix_GetError());
                    return 1;
                }

                //����������� ������ ���� 1 ���
                if (music) {
                    Mix_PlayMusic(Fmusic, 0);
                    music = false;
                }

                //�������������� ����������� ����� ���� ���������� ��������
                if (!Mix_PlayingMusic()) {
                    NowMusic++;
                    music = true;
                    Stop = true;
                    NextGb = true;
                    Position = 0;
                }

                LenMus = Mix_MusicDuration(Fmusic);   //���������� ����� �����
                LenMusMin = LenMus / 60;
                LenMusSec = LenMus - (LenMusMin * 60);
                //std::cout << LenMusMin << ":" << LenMusSec << std::endl;

                //���������� ������ � ������ ����
                int width, height;
                SDL_GetWindowSize(window, &width, &height);

                //������� �������� ����� �������� 35 ������
                MusText = c[NowMusic];
                std::string mp_delete{ ".mp3" };  // ������� ��������� .mp3
                size_t start1{ MusText.find(mp_delete) };            // ������� ������� ���������
                MusText.erase(start1, mp_delete.length());           //�������
                start1 = MusText.find(mp_delete, start1 + mp_delete.length());
                if (MusText.length() > 30) MusText.erase(30, 100);   //�������� �� 30 ��������
                MusText = (MusText + ".mp3");   //��������� � ����� ��������� .mp3
                std::string text = MusText;
                SDL_Surface* surface1 = TTF_RenderText_Solid(font, text.c_str(), textColor);
                SDL_Texture* texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
                int textWidth = surface1->w;
                int textHeight = surface1->h;
                SDL_Rect dstRect1 = { (width - textWidth) / 2 + (width / 6), (height - textHeight) / 2, textWidth, textHeight }; //������� � ���� ������ 
                SDL_RenderCopy(renderer, texture1, NULL, &dstRect1);

                //������� ��������� �����
                if (angle <= 128 && angle >= 0) VolProcD = (angle / 128) * 100;
                VolProc = std::to_string(VolProcD);
                VolProc.erase(2, 7); //�������� �� ����� 7 ��������� ������������� ����� 2 ��������
                std::string to_delete{ "," };  // ����� ��������� �������
                size_t start{ VolProc.find(to_delete) };            // ������� ������� ���������
                while (start != std::string::npos) // ������� � ������� ��� ��������� to_delete
                {
                    VolProc.erase(start, to_delete.length());
                    start = VolProc.find(to_delete, start + to_delete.length());
                }
                std::string textV = ("Volume " + VolProc + "%");
                SDL_Surface* surface2 = TTF_RenderText_Solid(font, textV.c_str(), textColor);
                SDL_Texture* texture2 = SDL_CreateTextureFromSurface(renderer, surface2);
                SDL_Rect dstRect2 = { (width - textWidth) / 2 + (width / 6), (height - textHeight) / 2 * 0.8, 150, 30 };
                SDL_RenderCopy(renderer, texture2, NULL, &dstRect2);
            }

            Mix_VolumeMusic(Volume);   //���������� ����������

            SDL_RenderPresent(renderer);

            SDL_Delay(1000 / 60);   //������������ FPS
        }
    }
    SDL_DestroyTexture(texture);
    //SDL_FreeSurface(surface);
    SDL_DestroyTexture(OnOff);
    SDL_DestroyTexture(BackG);
    SDL_DestroyTexture(PrevG);
    SDL_DestroyTexture(StopG);
    SDL_DestroyTexture(NextG);
    SDL_DestroyTexture(RewindBG);
    SDL_DestroyTexture(RewindFG);
    TTF_CloseFont(font);
    TTF_Quit();
    Mix_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}