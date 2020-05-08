//
//  main.cpp
//  Meedit
//
//  Created by René Descartes Domingos Muala on 09/04/20.
//  Copyright © 2020 Equal Team (René Descartes Domingos Muala). All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <string.h>
#include <ctype.h>

enum modes{Newfile, Normal};

int UsageMode, msg = 0, Change = 0;
bool ReadOnly = false, onhelp = false, sintax = true;
char UserName[1000],Path[10000],RPath[1000],Hfolder[10000],Cfolder[10000], NDPath[1000], NFile[1000];

struct simpleint {
    int pos;
};

struct simpleint hints[4];

WINDOW *win, *lwin;

void Process1();
int StartUp();
void Setup();
void edit(int a);
void Sprintw(WINDOW *win, const char *s, int at, int y);
void end(int exitStatus);

void resize(){
    int scry, scrx;
    getmaxyx(stdscr, scry, scrx);
    
    if(scry < 8 || scrx < 8){
        endwin();
        printf("\e[1m\e[91m E(1)\e[0m: \e[1mUnsupported screen size\e[0m\n");
        exit(1);
    }
    
    wresize(win, scry-1, scrx);
    wresize(lwin, 2,getmaxx(stdscr));
    
    wrefresh(win);
    wrefresh(lwin);
}

int YesOrNo(const char *s){
    curs_set(0);
    int ans = 1, a;
    msg = 1;
    char choices[2][10] = {"Yes","No"};
    lwin = newwin(2, getmaxx(stdscr), getmaxy(stdscr)-2, 0);
    keypad(lwin, true);
    do{
        wclear(lwin);
        wattron(lwin, COLOR_PAIR(3));
        mvwprintw(lwin, 1, 0," %s ", s);
        wattroff(lwin, COLOR_PAIR(3));
        for(int z = 0 ; z < 2 ; z ++){
            wprintw(lwin, " ");
            if(z == ans)
                wattron(lwin, A_STANDOUT);
            wattron(lwin, A_UNDERLINE);
            wprintw(lwin, "%s", choices[z]);
            wattroff(lwin, A_STANDOUT);
            wattroff(lwin, A_UNDERLINE);
        }
        wrefresh(lwin);
        a = wgetch(lwin);
        if(a == KEY_LEFT && ans > 0)
            ans--;
        else if(a == KEY_RIGHT && ans < 1)
            ans++;
        else if(a == 410)
            edit(410);
    } while(a != 10);
    curs_set(2);
    return ans;
}

void Warning(const char *s, char *s2){
    msg = 1;
    clear();
    attron(COLOR_PAIR(2));
    move(getmaxy(stdscr)-1,0);
    Sprintw(stdscr, s, 0, -2);
    attroff(COLOR_PAIR(2));
    if(s2 != NULL){
        attron(COLOR_PAIR(5));
        Sprintw(stdscr, s2, 0, -2);
        attroff(COLOR_PAIR(5));
    }
    refresh();
}

void Meedit(char *command){
    attron(A_BOLD);
    mvprintw(getmaxy(stdscr)-1,0,"-Meedit 0.2-");
    attroff(A_BOLD);
    if(ReadOnly == true)
        printw("(RO)");
    if(command != NULL)
        Sprintw(stdscr, command, 0, -2);
    refresh();
}

void scan(char *String) {
    while (*String) {
        if(*String == '\n')
            *String = NULL;
        String++;
    }
        
}

void Clearstring(char *String) {
    while (*String) {
        *String = NULL;
        String++;
    }
}

void cpstr(const char *from, char *to){
    
    Clearstring(to);
    
    while(*from){
        *to = *from;
        from++;
        to++;
    }
    *to = 10; //
    scan(to);
}

char* GetCut(int pos0, int pos1, const char *in){
    int a = 0;
    char buffer[100000], *bp;
    bp = buffer;
    
    while(*in && pos1 >= a){
        if(a >= pos0){
            *bp = *in;
            bp++;
        }
        in++;
        a++;
    }
    
    *bp = 10;
    scan(bp);
    
    bp = buffer;
    
    return bp;
}

void SStr(char *one, char *two, char *result){
    while (*one) {
        *result = *one;
        one++;
        result++;
    }
    while (*two) {
        *result = *two;
        two++;
        result++;
    }
    *result = 10; //
    scan(result);
}

int linesIn(char *fPath){
    int lines = 0;
    char buffer[100000];
    FILE *file = fopen(fPath, "rt");
    
    while(!feof(file)){
        fgets(buffer, 100000, file);
        lines++;
    }
    lines--;
    fclose(file);
    return lines;
}

bool istab(const char *str){
    bool tab = false;
    int a = 0, chances = 0, falls = 0;
    
    while (*str) {
        if(*str == ' ')
            chances++;
        else
            falls++;
        if(chances == 3 || falls > 0)
            break;
        str++;
        a++;
    }
    if(chances == 3 && falls == 0)
        tab = true;
    return tab;
}

bool has10(const char *str){
    bool has = false;
    
    while(*str){
        if(*str == 10)
            has = true;
        str++;
    }
    str = 0;
    return has;
}

void Exchange1(const char *in, char *ou){
    int pos = 0;
    
    bool _10 = has10(in);
    
    while(*in){
        if(istab(in) == true){
            *ou = 9;
            in+=2;
        } else {
            *ou = *in;
        }
        ou++;
        in++;
        pos++;
        
    }
    ou++;
    *ou = 10;
    scan(ou);
    
    if(_10 == true){
        ou++;
        //*ou = 10;
    }
}

void Exchange2(const char *in, char *ou){
    bool _10 = has10(in);
    while(*in){
        if(*in == 9){
            for(int z = 0; z < 3 ; z++){
                *ou = ' ';
                ou++;
            }
            ou--;
        } else {
            *ou = *in;
        }
        ou++;
        in++;
    }
    ou++;
    *ou = 10;
    scan(ou);
    
    if(_10 == true){
        ou++;
        //*ou = 10;
    }
}

int Mirror(int mode, char *fromfile, char *tofile){
    int status = 0;
    FILE *from, *to;
    int a = 0;
    char buffer[100000], buffer2[100000], MSG[50] = {"File cannot be openned"};
    
    from = fopen(fromfile, "rt");
    to = fopen(tofile, "wt");
    
    if(to == NULL){
        Warning(MSG, tofile);
        status = 1;
        goto jump;
    } else {
        fprintf(to, "%s", "");
        fclose(to);
    }

    to = fopen(tofile, "a");
    
    if(from == NULL){
        Warning(MSG, fromfile);
        goto jump;
    } else if(to == NULL){
        Warning(MSG, tofile);
        goto jump;
    }
    Clearstring(buffer);
    Clearstring(buffer2);
    while (a < linesIn(fromfile)) {
        fgets(buffer, 100000, from);
        if(mode == 1)
            Exchange1(buffer, buffer2);
        if(mode == 2)
            Exchange2(buffer, buffer2);
        if(mode == 0)
            fprintf(to, "%s", buffer);
        else
            fprintf(to, "%s", buffer2);
        a++;
        Clearstring(buffer);
        Clearstring(buffer2);
    }
jump:
    fclose(from);
    fclose(to);
    
    return status;
}

void Changeline(int line, char *string, bool endl){
    char tmpPath[10000], Copy[100000], buffer[100000];
    int n = 0, m = 0, lines = linesIn(Path);
    FILE *file01,*file02;
    
    /*setup*/{
        char tempFend[10] = {".CUT.00"};
        SStr(Path, tempFend, tmpPath);
    }
    
    /*clear*/{
        file01 = fopen(tmpPath, "wt");
        fprintf(file01, "%s", "");
        fclose(file01);
    }
    
    file01 = fopen(Path, "rt");
    file02 = fopen(tmpPath, "a");
    
    if(file01 == NULL || file02 == NULL){
        endwin();
        exit(3);
    }
    
    /*copy beffore(line)*/{
        while (!feof(file01)) {
            if(n == line || m == lines)
                break;
            fgets(Copy, 100000, file01);
            fprintf(file02, "%s", Copy);
            n++;
            m++;
        }
    }
    
    if(string == NULL){
        fprintf(file02, "%s", "\n");
        fgets(Copy, 100000, file01);
        fprintf(file02, "%s", Copy);
    } else {
        if(endl == true)
            fprintf(file02, "%s\n", string);
        else
            fprintf(file02, "%s", string);
        fgets(buffer, 100000, file01);
    }
    m++;
    /*copy after(line)*/{
        while (!feof(file01)) {
            if(m >= lines)
                break;
            fgets(Copy, 100000, file01);
            fprintf(file02, "%s", Copy);
            m++;
        }
    }
    
    fclose(file01);
    fclose(file02);
    
    /*clear*/{
        file01 = fopen(Path, "wt");
        fprintf(file01, "%s", "");
        fclose(file01);
    }
    
    file01 = fopen(Path, "a");
    file02 = fopen(tmpPath, "rt");
    
    m = 0;
    
    /*delivery*/{
        while (!feof(file02)) {
            if(m >= linesIn(tmpPath))
                break;
            fgets(Copy, 100000, file02);
            fprintf(file01, "%s", Copy);
            m++;
        }
    }
    fclose(file01);
    fclose(file02);
    
    /*remove*/{
        char command[100000], rm[4] = {"rm "};
        SStr(rm, tmpPath, command);
        system(command);
    }
}

bool IsIn(char a, char *b){
    bool is = false;
    
    while(*b){
        if(a == *b){
            is = true;
            break;
        }
        b++;
    }

    return is;
}

void Sprintw(WINDOW *window, const char *s, int at, int y){
    bool Norm_mode = true;
    bool Asp = false, single_cmnt = false;
    int a = 0, b = 0, c = 0;
    char numbers[11] = {"1234567890"}, operators[10] = {"+-*/="}, parentesis[10] = {"{}()[]"}, spchars[20] = {"<>~#@!&\\\'\"."}, ASPS[4] = {"\'\""}, until_s;
    
    if((int)strlen(s) > getmaxx(window) || at > 0){
        b = 1;
        c = 1;
    }
    
    if(y < 0){
        if(y == -1)
            y = 1;
        else
            y = 0;
        Norm_mode = false;
    }
    
    while(*s && b < getmaxx(window) && Norm_mode == true){
        if(a >= at){
            if(sintax == true && IsIn(*s, ASPS) == true){
                s--;
                until_s = *s;
                s++;
                if(until_s != '\\'){
                   if(Asp == true)
                       Asp = false;
                   else
                       Asp = true;
                }
                goto same;
            } else if(IsIn(*s, numbers) == true){
                s--;
                until_s = *s;
                s++;
                if(sintax == true && (IsIn(until_s, numbers) == true || IsIn(until_s, parentesis) == true || IsIn(until_s, spchars) == true || IsIn(until_s, operators) == true || until_s == ' ' || until_s == 10 || until_s == 27 || a == 0 ) && (Asp == false)){
                wattron(window, COLOR_PAIR(7));
                wprintw(window, "%c", *s);
                wattroff(window, COLOR_PAIR(7));
                } else {
                    goto same;
                }
            } else if(sintax == true && Asp == false && (IsIn(*s, parentesis) == true) && (Asp == false)){
                wattron(window, COLOR_PAIR(8));
                wprintw(window, "%c", *s);
                wattroff(window, COLOR_PAIR(8));
            } else if(sintax == true && (IsIn(*s, spchars) == true) && (Asp == false) ){
                wattron(window, COLOR_PAIR(1));
                wprintw(window, "%c", *s);
                wattroff(window, COLOR_PAIR(1));
            }else if(sintax == true && (IsIn(*s, operators) == true)){
                if(*s == '/'){
                    s--;
                    until_s = *s;
                    s++;
                    if(until_s == '/'){
                        single_cmnt = true;
                        wprintw(window, "/");
                    } else
                        goto same;
                } else
                    goto same;
            } else {
            same:
                if(Asp == true || single_cmnt == true)
                    wattron(window, COLOR_PAIR(1));
                wprintw(window, "%c", *s);
                if(Asp == false && single_cmnt == false)
                    wattroff(window, COLOR_PAIR(1));
            }
            b++;
        }
        s++;
        a++;
}
    while(*s && b < getmaxx(window) && Norm_mode == false){
        if(a >= at){
            wprintw(window, "%c", *s);
            b++;
        }
        s++;
        a++;
    }
    
    if(c == 1){
        wattron(window, A_STANDOUT);
        mvwprintw(window,y,getmaxx(win)-1,"+");
        wattroff(window, A_STANDOUT);
    }
    wattroff(window, COLOR_PAIR(1));
}

int see(int see,int line, char *ex, int &Allines,int ign, int ignchars){
    char toprint[100000],buffer[4] = {""},  Paths[100000], tpB[100000];
    strcpy(Paths, Path);
    int lines = 0, limit = linesIn(Paths), PageEnd = getmaxy(win), printed = 0;
    FILE *arch = fopen(Path, "rt");
    int y, x;
    for(int z = 0 ; z < ign ; z++){
        fgets(tpB, 100000, arch);
    }
    while(!feof(arch)){
        fgets(toprint, 100000, arch);
        if(see == true || see == 3){
            if(line == lines){
                if(see == true){
                    Sprintw(win, toprint, ignchars, line);
                } else
                    Sprintw(win, ex, ignchars, line);
            }
            else {
                getyx(win, y, x);
                Sprintw(win,toprint, 0,y);
            }
        }
        printed++;
        if(line == lines && see != 3){
            strcpy(ex, toprint);
        }         lines++;
        if(lines == limit || printed == PageEnd || printed+ign == limit)
            break;
    }
    
    if(line >= limit){
        strcpy(ex, buffer);
    }
    //
    Allines = limit;
    wrefresh(win);
    fclose(arch);
    return lines;
}

void ManageString(char command, char *String){
    static char buffer[200000];
    char *bufP = buffer;
    
    while (*bufP) {
        bufP++;
    }
    
    switch (command) {
        case 'a':
            while (*String) {
                *bufP = *String;
                bufP++;
                String++;
            }
            break;
        case 'n':
            *bufP = '\n';
            break;
        case 'g':
            strcpy(String, buffer);
            break;
        case 'c':
            Clearstring(buffer);
            break;
        default:
            break;
    }
}

int IsAB(char *Stra, char *Strb){
    int equalty = 0;
    while (*Stra && *Strb) {
        if(*Stra == *Strb){
            equalty++;
            Stra++;
        }
        Strb++;
    }
    return equalty;
}

int SpacesInA(char *Stra){
    int spaces = 0;
    while(*Stra){
        if(*Stra == ' ')
            spaces++;
        Stra++;
    }
    return spaces;
}


/// solving a mess
void es003BS(){
    edit(KEY_UP);
    edit(KEY_DOWN);
    edit(127);
}

/// enter / return
void es002(int curX, char *String, char *Line1, char *Line2){
    const char *Str = String;
    int b = 0;
    
    Clearstring(Line1);
    Clearstring(Line2);
    
    while(*Str){
        if(b < curX){
            *Line1 = *Str;
            Line1++;
        } else {
            *Line2 = *Str;
            Line2++;
        }
        Str++;
        b++;
    }
    
    *Line1 = 10; //
    *Line2 = 10; //
    
    scan(Line1);
    scan(Line2);
    
    //printw("~%s~ -> 1, ~%s~ -> 2", Line1, Line2); /* remember, that never will work, print outside the funtion */
}

/// delete a char
void es001(int curX, char *String){
    const char *SP = String;
    char Result[100000];
    char *RP = Result;
    int b = 0;
    
    if(curX < 0)
        curX = 0;
    
    while(*SP){
        if(b != curX){
            *RP = *SP;
            RP++;
        }
        SP++;
        b++;
    }
    
    *RP = 10; //
    
    cpstr(Result, String);
}

void savAtSP(){
    int my,mx = 0;
    curs_set(0);
    int a = 0, chars = 0, J = 0;
    msg = 1;
    char NEWPath[10000], arr[2][100] = {"~/", "/"};
    char *NewpP = NEWPath;
    lwin = newwin(2, getmaxx(stdscr), getmaxy(stdscr)-2, 0);
    char sat[20] = {" saved at "};
    wclear(lwin);
    wattron(lwin, COLOR_PAIR(4));
    mvwprintw(lwin, 0, 0," %s ", " Where? ");
    wattroff(lwin, COLOR_PAIR(4));
    do{
        getyx(lwin, my, mx);
        NewpP = NEWPath;
        a = wgetch(lwin);
        if(a == 410){
            resize();
        } else if(a == 27){
            edit(410);
            goto endf;
        }else if(a == 127 || a == KEY_BACKSPACE){
            if(strlen(NewpP) > 0){
            while (*NewpP){
                NewpP++;
            }
            NewpP--;
            *NewpP = NULL;
                NewpP++;
                *NewpP = 10;
                scan(NewpP);
            chars--;
                if(J > 0)
                    J--;
            }
        } else{
            chars++;
            while (*NewpP) {
                NewpP++;
            }
            *NewpP = (char)a;
            NewpP++;
            *NewpP = 10;
            scan(NewpP);
            if( mx + 1== getmaxx(lwin))
                J++;
        }
        if(strcmp(NEWPath, arr[0]) == 0){
            SStr(Hfolder, arr[1], NEWPath);
            chars = (int)strlen(Hfolder) + 1;
        }
        wclear(lwin);
        wattron(lwin, COLOR_PAIR(4));
        mvwprintw(lwin, 0, 0," %s ", " Where? ");
        wattroff(lwin, COLOR_PAIR(4));
        wmove(lwin, 1, 0);
        Sprintw(lwin, NEWPath, J, -1);
        refresh();
        
    } while(a != 10);
    scan(NEWPath);
    if(Mirror(1, Path, NEWPath) == 0){
        Warning(sat, NEWPath);
        Change = 0;
    }
endf:
    curs_set(2);
    Clearstring(NEWPath);
}

void saveAt(){
    char ro[100] = {" This is a RO file... "};
    if(onhelp == false && ReadOnly == false){
        edit(410);
        if(onhelp == false)
        savAtSP();
    } else
        Warning(ro, NULL);
}

void save(){
    char sa[10] = {" saved "}, ro[100] = {" This is a RO file... "};
    if(onhelp == false && ReadOnly == false){
        edit(410);
        if(UsageMode == Normal){
            Mirror(1, Path, RPath);
            Warning(sa, NULL);
            Change = 0;
        }
    } else
        Warning(ro, NULL);
}

void fquit(){
    endwin();
    exit(0);
}

void quit(){
    char Message[20] = {" Unsaved! "};
    if(Change == 0){
        end(0);
    } else {
        edit(410);
        Warning(Message, NULL);
    }
}


void delAfileSP(char *filePath){
    char rm[4] = {"rm "}, command[1000];
    SStr(rm, filePath, command);
    system(command);
}

void deleteAfile(){
    if(onhelp == false){
        char name[10] = {" delete? "};
        if(YesOrNo(name) == 0) {
            Change = 0;
            delAfileSP(Path);
            if(UsageMode == Normal)
                delAfileSP(RPath);
            Process1();
            StartUp();
        }
    }
}

void close2nd(){
    ReadOnly = false;
    onhelp = false;
    if(strlen(NDPath)>0){
        cpstr(NDPath, Path);
        Clearstring(NDPath);
    }
    else {
        endwin();
        exit(0);
    }
}

void help(){
    char HFileID[30] = {"Help.document"}, HelpFile[1000];
    ReadOnly = true;
    if(onhelp == false){
        cpstr(Path, NDPath);
        SStr(Cfolder, HFileID, HelpFile);
        cpstr(HelpFile, Path);
        onhelp = true;
    }
}

void Orthographic(int a, char *string){
    static char myphrase[100000];
    char *mpP = myphrase;
    static int H = 0, selected = 0;
    int b = 0;
    lwin = newwin(2, getmaxx(stdscr), getmaxy(stdscr)-2, 0);
    
        char Obase[8][10] = {"","help","save","save at","close","quit", "fquit","delete"};
    
    if(H == 0)
        selected = 0;
    
    while (*mpP) {
        mpP++;
    }
    
    if(strlen(myphrase)>0){
        b = (int)strlen(myphrase);
    } else
        b = 0;
    
    if(a == KEY_SLEFT || a == KEY_BACKSPACE || a == 127){
        mpP--;
        *mpP=NULL;
        H = 0;
    } else if (a == KEY_LEFT){
        selected--;
        if(selected < 0)
            selected++;
        goto dontw;
    } else if (a == KEY_RIGHT){
        selected++;
        if(selected >= H)
            selected = H - 1;
        goto dontw;
    } else if (a == ERR){
        goto dontw;
    } else if(a == 10){
        H = 0;
        Clearstring(myphrase);
        strcpy(myphrase, Obase[hints[selected].pos]);
        strcpy(string, myphrase);
        Clearstring(myphrase);
    } else if (a == 27 ){
        Clearstring(myphrase);
        goto dontw;
    }else if(a == 410){
        goto dontw;
    } else{
        H = 0;
        a = tolower(a);
        *mpP= (char)a;
        mpP++;
    }
    
    for (int z = 0 ; z < 8 ; z ++) {
        if(IsAB(myphrase, Obase[z])>=1){
            hints[H].pos = z;
            if(strcmp(myphrase, Obase[z])==0){
                selected = H;
                break;
            }
            else if(IsAB(myphrase, Obase[z])>1 && (IsAB(myphrase, Obase[z])) == strlen(myphrase))
                selected = H;
            else
                selected = 0;
            H++;
            if(H == 3)
                break;
        } else {
            hints[H].pos = 0;
        }
    }
    
dontw:
    wclear(lwin);
    //whline(lwin, 0, getmaxx(stdscr));
    wmove(lwin,0,0);
        Sprintw(lwin, Path, 0, -2);
        //mvwprintw(lwin,0,0, "[%s]", Path);
        mvwprintw(lwin,1,1, "==> %s ", myphrase);
    if(H>0)
        wprintw(lwin, ":");
    for (int z = 0 ; z < H; z ++) {
        wprintw(lwin ," ");
        if(z == selected)
            wattron(lwin, A_REVERSE);
        wattron(lwin, A_UNDERLINE);
            wprintw(lwin, "%s", Obase[hints[z].pos]);
        wattroff(lwin, A_REVERSE);
        wattroff(lwin, A_UNDERLINE);
    }
    wrefresh(lwin);
    wrefresh(win);
}

void CommandLine(int &lY, int &lX, int &ign, bool recover){
    Orthographic(ERR, NULL);
    static int BkpL[3];
    char Command[100];
    int a = 0;
    bool recovery = true;
    curs_set(0);
    if(onhelp == false){
        BkpL[0] = lY;
        BkpL[1] = lX;
        BkpL[2] = ign;
    }
    lY = lX = ign = -1;
    wattron(win, A_BOLD);
    mvwprintw(win,getmaxy(stdscr)-3,0,"- MCL -");
    wattroff(win, A_BOLD);
    do{
        if(a == 410){
            sintax = true;
            edit(410);
            sintax = false;
        }
        a = wgetch(win);
        Orthographic(a, Command);
        if(a == 27)
            a = 10;
    }while(a != 10);
    curs_set(2);
    if(strcmp(Command, "help")==0){
        sintax = false;
        lY = lX = ign = 0;
        help();
        if(onhelp == true){
            ReadOnly = true;
        }
        recovery = false;
    }
    if(strcmp(Command, "close")==0){
        close2nd();
        if(onhelp == true)
            onhelp = false;
    }
    if(strcmp(Command, "delete")==0){
        deleteAfile();
        recovery = false;
    }
    if(strcmp(Command, "fquit")==0){
        fquit();
    }
    
    if(strcmp(Command, "quit")==0){
        quit();
    }
    
    if(strcmp(Command, "save")==0){
        save();
    }
    
    if(strcmp(Command, "save at")==0){
        saveAt();
    }
    
    if(recovery == true){
        if(onhelp == false){
            lY = BkpL[0];
            lX = BkpL[1];
            ign = BkpL[2];
            sintax = recover;
        }
    }
    
    if(strcmp(Command, "save")!=0 && strcmp(Command, "quit")!= 0 && strcmp(Command, "save at")!= 0){
        edit(410);
    }
    
}

int FOLINE(char mode, char *str){
    static char Loaded[100000];
    
    switch(mode){
        case 'w':
            strcpy(Loaded, str);
            break;
        case 'g':
            strcpy(str, Loaded);
            break;
        case 'c':
            Clearstring(Loaded);
            break;
        default:
            break;
    }
    return (int)strlen(Loaded);
}

void editSup(int limit, int a ,int &lY, int &curX, char *line, int allines, int &ign, int &ignchars){
    const char *lb = line;
    char buffer[100000], B2[100000], B3[100000], Line1[100000], Line2[100000], zero[4] = {""};
    char *bp = buffer;
    Clearstring(buffer);
    int b = 0;
    clear();
    
    if(a != 127 && a != 10 && a != ERR){
        while (*lb) {
            if(b != curX+ignchars){
                *bp = *lb;
                lb++;
            }
            else {
                *bp = (char)a;
            }
            bp++;
            b++;
        }
        
        if(curX+ignchars >= strlen(line)){
            *bp = (char)a;
            bp++;
        }
        
        *bp = 10;
        
        if(curX + 1 < getmaxx(win))
            curX++;
        else
            ignchars++;
        
    } else if (a == 127){
        if(lY == 0 && ign > 0){
            es003BS();
            goto end;
        } else if(curX == 0 && lY != 0 && ignchars == 0){
            FOLINE('c', NULL);
            {
                cpstr(line, buffer);
                scan(buffer);
                Changeline(lY+ign, buffer, true);
            }
            cpstr(line, buffer);
                lY--;
            see(false, lY, B2, allines, ign, 0);
            scan(B2);
            SStr(B2, buffer, B3);
            scan(B3);
            strcpy(buffer, B3);
            Changeline(lY+ign, buffer, true);
            Changeline((lY+1)+ign, zero, false);
            if(strlen(B2) < getmaxx(win))
                curX = (int)strlen(B2);
            else{
                curX = getmaxx(win) - (int)strlen(line) - 1;
                ignchars = (int)strlen(buffer) - getmaxx(win) + 2;
                if(ignchars > 0)
                    ignchars--;
            }
            goto end;
        } else {
            es001((curX+ignchars)-1, line);
            cpstr(line, buffer);
            scan(buffer);
            if(ignchars > 0)
                ignchars--;
            else
                curX--;
        }
    } else if(a == 10 || a == '\n'){
        FOLINE('c', NULL);
        {
            cpstr(line, buffer);
            scan(buffer);
            Changeline(lY+ign, buffer, true);
        }
        es002(curX+ignchars, line, Line1, Line2);
        Changeline(lY+ign, Line1, true);
        if(lY + 1 < getmaxy(win))
            lY++;
        else {
            ign++;
        }
        if(lY+ign < allines)
            Changeline(lY+ign, NULL, true);
        else if(lY+ign >= allines){
            Changeline(lY+ign, zero, true);
        }
        
        if(strlen(Line2) > 0){
            Changeline(lY+ign, Line2, true);
        }
        curX = 0;
        goto end;
    } else if(a == ERR){
        FOLINE('c', NULL);
        cpstr(line, buffer);
        Changeline(lY+ign, buffer, true);
        goto end;
    }
    
    scan(buffer);
    FOLINE('w', buffer);
end:
    printw("");
}

void edit(int a){
    char ex[100000], extp[100000], exb[100000], NL[4] = {"\n"};
    static char lastex[100000];
    static int lY = 0, lX = 0,ign = 0, lines = 0, allines, ignchars = 0, igNBug, lastline, llY, lign, Bline = -1, LChan;
    int linemv = 0;
    static bool saveline = false;
    static bool Firstime = true, ischan = false, canload, isloaded = false;
   
    lastline = lY + ign;
    llY = lY;
    lign = ign;
    strcpy(lastex, ex);
    LChan = Change;
restart:
    igNBug = ignchars;
    if(a == KEY_LEFT){
        if( lX - 1 >= 0)
            lX--;
        else if (lY - 1 >= 0 && ignchars == 0){
            edit(KEY_UP);
            see(false,lY, ex,allines, ign, 0);
            scan(ex);
            if(strlen(ex) < getmaxx(win))
                lX = (int)strlen(ex);
            else{
                lX = getmaxx(win) - 1;
                ignchars = (int)strlen(ex) - getmaxx(win) + 1;
            }
            Clearstring(ex);
        } else if(strlen(ex) > getmaxx(win) && ignchars > 0){
            ignchars--;
        }
    } else if(a == KEY_RIGHT){
        if( lX + 1 < getmaxx(win) && lX < strlen(ex))
            lX++;
        else if(strlen(ex) > getmaxx(win) && ignchars + getmaxx(win) <= strlen(ex)){
            ignchars++;
        }
        else if(lY + 1 < getmaxy(win) && lY + 1 < lines){
            lX = 0;
            edit(KEY_DOWN);
        }
    } else if(a == KEY_UP){
        ignchars = 0;
        if(lY - 1 >= 0){
            lY--;
        } else {
            if(ign > 0)
                ign--;
        }
    }  else if(a == KEY_DOWN){
        ignchars = 0;
        if(lY + 1 < getmaxy(win) && lY + 1 < lines)
            lY++;
        else {
            if(ign+lines < allines)
                ign++;
        }
    } else if (a == ERR) {
        //wprintw(win,"ERR");
    } else if (a == 410) {
    CLEAR:
        clear();
        resize();
        Meedit(NULL);
    } else if (a == 27) {
        static bool recover = false;
        if(onhelp == false){
            recover = sintax;
            sintax = false;
            FOLINE('g', exb);
            editSup(lines, ERR, lY ,lX, exb, allines, ign,ignchars);
        }
        CommandLine(lY, lX, ign, recover);
        isloaded = false;
        if(onhelp == false)
            sintax = recover;
    } else if (a == 9) {
        if(ReadOnly == false && onhelp == false){
            for(int z = 0 ; z < 3 ; z ++){
                editSup(lines, ' ', lY,lX, ex, allines, ign,ignchars);
                FOLINE('g', ex);
            }
        }
    }else {
        if(ReadOnly == false && onhelp == false){
            editSup(lines,a, lY,lX, ex, allines, ign,ignchars);
            Change = 1;
            ischan = true;
            if(a == 10)
                ignchars = 0;
            if(msg == 1){
                msg = 0;
                goto CLEAR;
            }
        }
    }
    
    wclear(win);
    
    if(lX < 0)
        lX = 0;
    if(lY < 0)
        lY = 0;
    
    if(lastline != lY + ign){
        linemv = lY+ign - lastline;
        if(ischan == true && a != 127 && a != 10)
            saveline = true;
        else saveline = false;
        ischan = false;
        isloaded = false;
        
        Bline = -1;
        canload = true;
    } else {
        linemv = 0;
        canload = false;
    }
    
    wattron(win, COLOR_PAIR(1));
    wmove(win, 10, 0);
    
    //saveline == true && a != 127 && a != 10
     
    if(saveline == true && ReadOnly == false && onhelp == false){
        FOLINE('g', exb);
        editSup(lines, ERR,  llY ,lX, exb, allines, lign,ignchars);
        isloaded = false;
        saveline = false;
    }
    
    wmove(win, 0, 0);
    wattroff(win, COLOR_PAIR(1));
    
    //wattron(win, COLOR_PAIR(1));
    if(isloaded == false || ReadOnly == true){
        Clearstring(ex);
        lines = see(true,lY, ex,allines, ign, ignchars);
        canload = true;
    } else {
        Clearstring(exb);
        FOLINE('g', ex);
        ManageString('c', NULL);
        ManageString('a', ex);
        ManageString('a', NL);
        ManageString('g', extp);
        ManageString('c', NULL);
        lines = see(3,Bline, extp,allines, ign, ignchars);
    }
    scan(ex);
    //wattroff(win, COLOR_PAIR(1));
    if(onhelp == false && ReadOnly == false){
        if(strlen(ex) < getmaxx(win)){
            ignchars = 0;
        }
        
        if(strlen(ex) < getmaxx(win) && igNBug > 0){
            if(lX + 1 == getmaxx(win))
                goto normlX;
        }
        
        if(lX > strlen(ex)){
        normlX:
            lX = (int)strlen(ex);
        }
        
        if(canload == true || Firstime == true){
            FOLINE('w', ex);
            isloaded = true;
            Bline = lY;
            Firstime = false;
        }
    }
    
    //wprintw(win,"\"%d\" -- tabsize", TABSIZE);
    //mvwprintw(win,30, 1,"line: %d | coln: %d", lY,lX);
    //mvw printw(win,30, 1,"line: %d<%d | coln: %d | IGN: %d | realine: %d | R.line %d>%d", lY,allines, lX, ign, lY+ign, lines, getmaxy(win));
    //wprintw(win,"\"%s\" -- ex", ex);
    //mvwprintw(win,31, 1,"a : %c : %d", a, a);
    wmove(win,lY, lX);
    wrefresh(win);
}


void Setup(){
    int scry, scrx;
    getmaxyx(stdscr, scry, scrx);
    win = newwin(scry-3,scrx, 0, 0);
    keypad(win, true);
    edit(410);
    
    int a = 0;
    while(1){
       a = wgetch(win);
        if(a != ERR){
            edit(a);
        }
    }
}

void Process1(){
    char tag1[10] = {"/Users/"}, tag01[10] = {"/"}, tag2[100] = {"/.technolandia_meedit/"},tag3[100]={"unsaved.document"}, buffer[10000];
    system("id -un > //tmp/technolandia_meedit.tf.01");
    
    FILE *un = fopen("/tmp/technolandia_meedit.tf.01", "rt");
    if(un == NULL){
        printf("\e[1mMeedit:\e[0m \"//tmp/...\"\n");
        endwin();
        exit(4);
    }
    fgets(UserName, 1000, un);
    scan(UserName);
    fclose(un);
    
    system("rm //tmp/technolandia_meedit.tf.01");
    
    ManageString('c', NULL);
    if(strcmp(UserName, "root")!=0)
        ManageString('a', tag1);
    else {
        ManageString('a', tag01);
    }
    ManageString('a', UserName);
    ManageString('g', Hfolder);
    ManageString('a', tag2);
    ManageString('g', Cfolder);
    ManageString('a', tag3);
    ManageString('g', buffer);
    ManageString('c', NULL);
    cpstr(buffer, Path);
}

int StartUp(){
    UsageMode = Newfile;
    int status = 0;
    
    FILE *test;
        test = fopen(Path, "wt");
        if(test == NULL){
            printf("\e[1mMeedit:\e[0m \"%s\"not found\n", Path);
            status = 1;
        } else
            fprintf(test, "%s", " ");
    
    fclose(test);
    
    return status;
}

void StartUp2(){
    char unsaved[50] = {".unsaved"};
    FILE *test = fopen(RPath, "rt");
    
    if(test == NULL){
        fclose(test);
        test = fopen(RPath, "wt");
        if(test == NULL){
            fclose(test);
            endwin();
            printf("\e[1mMeedit:\e[0m File cannot be opened\n");
            exit(3);
        }
    }
    
    SStr(RPath, unsaved, Path);
    Mirror(2, RPath, Path);
}

void end(int exitStatus){
    if(onhelp == false){
        delAfileSP(Path);
    }
    endwin();
    exit(exitStatus);
}

int main(int agrc, char **argv) {
    initscr();
    cbreak();
    noecho();
    
    start_color();
    use_default_colors();
    
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, -1, COLOR_YELLOW);
    init_pair(3, -1, COLOR_RED);
    init_pair(4, -1, COLOR_GREEN);
    init_pair(5, -1, COLOR_WHITE);
    init_pair(6, COLOR_RED, -1);
    init_pair(7, COLOR_GREEN, -1);
    init_pair(8, COLOR_YELLOW, -1);
    init_pair(9, COLOR_MAGENTA, -1);
    init_pair(10, COLOR_BLUE, -1);
    
    Process1();
    UsageMode = Normal;
    
    if(agrc <= 1){
        if(StartUp()==1)
            exit(3);
    } else {
        cpstr(argv[1], RPath);
        StartUp2();
    }
    
    Setup();
    
    getch();
    endwin();
    return 0;
}
