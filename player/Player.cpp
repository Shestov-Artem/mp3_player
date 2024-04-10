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

    SDL_Window* window = SDL_CreateWindow("ARTEMOPLEER", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 500, 0);                       //размер окна не меняется
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    //получаем путь к папке, где находится исполняемый файл
    wchar_t path[MAX_PATH];
    GetCurrentDirectory(sizeof(path), path);
    //wprintf(L"%s\n", path);

    //преобразуем path типа wchar_t в тип char*, а затем в string
    char* pcstr = (char*)malloc(sizeof(char) * (2 * wcslen(path) + 1));
    memset(pcstr, 0, 2 * wcslen(path) + 1);
    w2c(pcstr, path, 2 * wcslen(path) + 1);
    std::string a = pcstr;
    std::cout << "Путь к папке с приложением: " << a << std::endl;
    free(pcstr);

    //смотрим количество файлов mp3 в нашей папке
    const std::string folderPath = a;
    int  KolMusic = 0;
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (fs::is_regular_file(entry.path()) && entry.path().extension() == ".mp3") {
            KolMusic++;
        }
    }
    std::cout << "Найдено " << KolMusic << " файла mp3:" << std::endl;

    //music
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL Error: ");
        return 1;
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: ");
        return 1;
    }

    //создаем массив из названий песен из нашей папки
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

    //кнопки управления 
    SDL_Texture* BackG = LoadImage("BackGroundPleer.png"); //задний фон
    SDL_Texture* StopG = LoadImage("PlayStop.png");
    SDL_Texture* PrevG = LoadImage("Prev.png");
    SDL_Texture* NextG = LoadImage("Next.png");
    SDL_Texture* RewindFG = LoadImage("RewindForward.png");
    SDL_Texture* RewindBG = LoadImage("RewindBack.png");
    SDL_Texture* OnOff = LoadImage("OnOff.png");

    SDL_Surface* surface = IMG_Load("VolCircle.png");      //колесо управления громкостью
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    // Определение прямоугольника для исходного изображения (задаем зону исходной картинки которую хотим взять)
    SDL_Rect srcRect = { 0, 0, 500, 500 };
    // Определение прямоугольника для целевого изображения (вроде нач координаты и размер)
    SDL_Rect dstRect = { 150, 195, 110, 110 };
    double angle = 45;    // Угол поворота в градусах

    //Выводим на экран название песни
    TTF_Font* font = TTF_OpenFont("Machine.ttf", 17);
    if (!font) {
        std::cout << "Невозможно уствновить заданный шрифт";
        return 1;
    }
    SDL_Color textColor = { 84, 255, 159 }; // White color: 255,255,255

    bool music = true; //проигрываем в цикле песню 1 раз
    int NowMusic = 0;  //номер текущего трека
    bool Stop = true;  //пауза
    int Volume = 32;   //громкость музыки, максимум 128
    double Position = 0;  //текущее время перемотки песни 

    bool VolmGb = false;  //флажки для нажатия кнопок клавишами
    bool VolpGb = false;
    bool NextGb = false;
    bool PrevGb = false;
    bool StopGb = false;

    std::string VolProc;  //текст для вывода громкости
    double VolProcD;      //громкосьт в процентах

    std::string MusText;  //названия песен

    bool NextButton = false;  //true если курсор находиться на нужной кнопке
    bool PrevButton = false;
    bool StopButton = false;

    bool Rewind10secButton = false;  //анимация нажатия
    bool Rewind10sec = false;        //true если курсор находиться на нужной кнопке
    bool RewindBackButton = false;
    bool RewindBack = false;

    double LenMus; //длина музыки в секундах
    int LenMusMin; //длина музыки в полных минутах
    int LenMusSec; //оставшиеся секунды

    bool On = true;
    bool OnGb = false;
    bool OnButton = false;

    int close = 0;
    while (!close) {// animation loop
        SDL_Event event;
        while (SDL_PollEvent(&event)) {// Управление событиями
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
                // управление кнопкой закрытия
                close = 1;
                break;
            case SDL_KEYDOWN:
                // API клавиатуры для определения нажатой клавиши
                switch (event.key.keysym.scancode) {
                case SDL_SCANCODE_LEFT:
                    NowMusic--;  //листать песни
                    music = true; //каждый раз даем возможность проиграть файл 1 раз
                    Stop = true;  //чтобы остановить трек если даже переключили его
                    PrevGb = true; //анимация нажатия кнопки
                    Position = 0;  //обнуляем перемотку
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
                    Mix_RewindMusic();   //проигрывание трека с начала
                    RewindBackButton = true;
                    Position = 0;
                    break;
                case SDL_SCANCODE_W:
                    Position = Position + 10.0;
                    Mix_SetMusicPosition(Position);   //перемотка вперед на 10 секунд
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
                // API клавиатуры для определения нажатой кнопки мыши
                switch (event.button.button) {
                case SDL_BUTTON_LEFT:
                    if (NextButton) {
                        NowMusic++;
                        music = true;  //проиграть музыку 1 раз
                        Stop = true;   //чтобы стоп работал при переключении песни
                        NextGb = true;  //анмация нажатия кнопки
                        Position = 0;  //обнуляем перемотку
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
                        Mix_SetMusicPosition(Position);   //перемотка вперед на 10 секунд
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
            //координаты мыши
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            SDL_RenderClear(renderer);

            SDL_RenderCopy(renderer, BackG, NULL, NULL);

            //управление музыкой мышкой
            //листать и останавливать треки
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
            //перемотка
            if (mouseX > 290 && mouseX < 360 && mouseY > 170 && mouseY < 200) {  //перемотка на 10 сек вперед
                Rewind10sec = true;
                if (Rewind10secButton) SDL_RenderCopy(renderer, RewindFG, NULL, NULL);
            }
            else Rewind10sec = false;
            if (mouseX > 290 && mouseX < 360 && mouseY > 300 && mouseY < 325) {  //перемотка в начало трека
                RewindBack = true;
                if (RewindBackButton) SDL_RenderCopy(renderer, RewindBG, NULL, NULL);
            }
            else RewindBack = false;
            if (mouseX > 873 && mouseX < 932 && mouseY > 125 && mouseY < 150) {  //Вкл/Выкл плеер
                OnButton = true;
                if (OnGb) SDL_RenderCopy(renderer, OnOff, NULL, NULL);
            }
            else OnButton = false;

            //анимации нажатия кнопок с клавиатуры
            if (RewindBackButton) SDL_RenderCopy(renderer, RewindBG, NULL, NULL);
            if (Rewind10secButton) SDL_RenderCopy(renderer, RewindFG, NULL, NULL);
            if (StopGb) SDL_RenderCopy(renderer, StopG, NULL, NULL);
            if (NextGb) SDL_RenderCopy(renderer, NextG, NULL, NULL);
            if (PrevGb) SDL_RenderCopy(renderer, PrevG, NULL, NULL);
            if (OnGb) SDL_RenderCopy(renderer, OnOff, NULL, NULL);

            //управление колесом
            if (VolmGb && angle > 0) angle--;
            if (VolpGb && angle < 128) angle++;
            SDL_RenderCopyEx(renderer, texture, &srcRect, &dstRect, angle, NULL, SDL_FLIP_NONE);

            if (StopGb) SDL_RenderCopy(renderer, StopG, NULL, NULL);   //анимация нажатия колесика Play Stop

            //условия проверки номера музыкального файла 
            if (NowMusic >= KolMusic)
                NowMusic = 0;
            if (NowMusic < 0)
                NowMusic = KolMusic - 1;

            if (On) { //если плеер включен
                Mix_Music* Fmusic = Mix_LoadMUS(c[NowMusic]);
                if (!Fmusic) {
                    SDL_Log("Unable to load music file: %s", Mix_GetError());
                    return 1;
                }

                //проигрываем нужный файл 1 раз
                if (music) {
                    Mix_PlayMusic(Fmusic, 0);
                    music = false;
                }

                //автомотическое перключение трека если предыдущий кончился
                if (!Mix_PlayingMusic()) {
                    NowMusic++;
                    music = true;
                    Stop = true;
                    NextGb = true;
                    Position = 0;
                }

                LenMus = Mix_MusicDuration(Fmusic);   //определяем длину песни
                LenMusMin = LenMus / 60;
                LenMusSec = LenMus - (LenMusMin * 60);
                //std::cout << LenMusMin << ":" << LenMusSec << std::endl;

                //определяем ширину и высоту окна
                int width, height;
                SDL_GetWindowSize(window, &width, &height);

                //Выводим название песни максимум 35 знаков
                MusText = c[NowMusic];
                std::string mp_delete{ ".mp3" };  // удаляем подстроку .mp3
                size_t start1{ MusText.find(mp_delete) };            // находим позицию подстроки
                MusText.erase(start1, mp_delete.length());           //удаляем
                start1 = MusText.find(mp_delete, start1 + mp_delete.length());
                if (MusText.length() > 30) MusText.erase(30, 100);   //обрезаем до 30 символов
                MusText = (MusText + ".mp3");   //добавляем в конец удаленный .mp3
                std::string text = MusText;
                SDL_Surface* surface1 = TTF_RenderText_Solid(font, text.c_str(), textColor);
                SDL_Texture* texture1 = SDL_CreateTextureFromSurface(renderer, surface1);
                int textWidth = surface1->w;
                int textHeight = surface1->h;
                SDL_Rect dstRect1 = { (width - textWidth) / 2 + (width / 6), (height - textHeight) / 2, textWidth, textHeight }; //выводим в окно плеера 
                SDL_RenderCopy(renderer, texture1, NULL, &dstRect1);

                //выводим громкость песни
                if (angle <= 128 && angle >= 0) VolProcD = (angle / 128) * 100;
                VolProc = std::to_string(VolProcD);
                VolProc.erase(2, 7); //отрезаем от числа 7 элементов расположенных после 2 элемента
                std::string to_delete{ "," };  // какую подстроку удалить
                size_t start{ VolProc.find(to_delete) };            // находим позицию подстроки
                while (start != std::string::npos) // находим и удаляем все вхождения to_delete
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

            Mix_VolumeMusic(Volume);   //управление громкостью

            SDL_RenderPresent(renderer);

            SDL_Delay(1000 / 60);   //ограничиваем FPS
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