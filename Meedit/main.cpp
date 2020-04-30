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

//#define bPath "//Users/crses/Meedit/tmp/Meedit.unsaved.txt"
//#define EPath "//Users/crses/Meedit/Example.txt"

enum modes{Newfile, Normal};

int UsageMode, msg = 0, Change = 0;
bool ReadOnly = false, onhelp = false;
char UserName[1000],Path[10000],RPath[1000],Hfolder[10000],Cfolder[10000], NDPath[1000], NFile[1000];

struct simpleint {
    int pos;
};

struct simpleint hints[4];

WINDOW *win, *lwin;


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
    Sprintw(stdscr, s, 0, 0);
    attroff(COLOR_PAIR(2));
    if(s2 != NULL){
        attron(COLOR_PAIR(5));
        Sprintw(stdscr, s2, 0, 0);
        attroff(COLOR_PAIR(5));
    }
    refresh();
}

void Meedit(char command){
    attron(A_BOLD);
    mvprintw(getmaxy(stdscr)-1,0,"-Meedit 0.1-");
    attroff(A_BOLD);
    if(ReadOnly == true)
        printw("(RO)");
    switch (command) {
        case '>':
            printw(" ");
            attron(A_STANDOUT);
            printw(" >> ");
            attroff(A_STANDOUT);
            printw(" ");
            break;
        case '<':
            printw(" ");
            attron(A_STANDOUT);
            printw(" << ");
            attroff(A_STANDOUT);
            printw(" ");
            break;
        case '=':
            printw(" ");
            attron(A_STANDOUT);
            printw(" << ");
            attroff(A_STANDOUT);
            printw(" ");
            attron(A_STANDOUT);
            printw(" >> ");
            attroff(A_STANDOUT);
            printw(" ");
            break;
        default:
            break;
    }
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

int Mirror(char *fromfile, char *tofile){
    int status = 0;
    FILE *from, *to;
    int a = 0;
    char buffer[100000], MSG[50] = {"File cannot be openned"};
    
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
    while (a < linesIn(fromfile)) {
        fgets(buffer, 100000, from);
        fprintf(to, "%s", buffer);
        a++;
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

void Sprintw(WINDOW *window, const char *s, int at, int y){
    int a = 0, b = 0, c = 0;
    if((int)strlen(s) > getmaxx(window) || at > 0){
        b = 1;
        c = 1;
    }
    
    while(*s && b < getmaxx(window)){
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
}

int see(bool see,int line, char *ex, int &Allines,int ign, int ignchars){
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
        if(see == true){
            if(line == lines){
                Sprintw(win, toprint, ignchars, line);
            }
            else {
                getyx(win, y, x);
                Sprintw(win,toprint, 0,y);
            }
        }
        printed++;
        if(line == lines){
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
        Sprintw(lwin, NEWPath, J, 1);
        refresh();
        
    } while(a != 10);
    scan(NEWPath);
    if(Mirror(Path, NEWPath) == 0)
    Warning(sat, NEWPath);
endf:
    curs_set(2);
    Clearstring(NEWPath);
}

void saveAt(){
    edit(410);
    savAtSP();
    Change = 0;
}

void save(){
    char sa[10] = {" saved "};
    edit(410);
    if(UsageMode == Normal){
        Mirror(Path, RPath);
        Warning(sa, NULL);
        Change = 0;
    } else
        saveAt();
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


void delAfileSP(){
    char rm[4] = {"rm "}, command[1000];
    SStr(rm, Path, command);
    system(command);
}

void deleteAfile(){
    if(onhelp == false){
        
        char name[10] = {"delete?"};
        if(YesOrNo(name) == 0)
            delAfileSP();
        StartUp();
        Setup();
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
        mvwprintw(lwin,0,0, "[%s]", Path);
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

void CommandLine(int &lY, int &lX, int &ign){
    Orthographic(ERR, NULL);
    static int BkpL[3];
    char Command[100];
    int a = 0;
    curs_set(0);
    wattron(win, A_BOLD);
    mvwprintw(win,getmaxy(stdscr)-3,0,"- MCL -");
    wattroff(win, A_BOLD);
    do{
        if(a == 410){
            edit(410);
        }
        a = wgetch(win);
        Orthographic(a, Command);
        if(a == 27)
            a = 10;
    }while(a != 10);
    curs_set(2);
    if(strcmp(Command, "help")==0){
        if(onhelp == false){
            BkpL[0] = lY;
            BkpL[1] = lX;
            BkpL[2] = ign;
        }
        lY = lX = 0;
        help();
        if(onhelp == true){
            ReadOnly = true;
        }
    }
    if(strcmp(Command, "close")==0){
        close2nd();
        lY = BkpL[0];
        lX = BkpL[1];
        ign = BkpL[2];
    }
    if(strcmp(Command, "delete")==0){
        lY = lX = ign = 0;
        deleteAfile();
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
    
    if(strcmp(Command, "save")!=0 && strcmp(Command, "quit")!= 0 && strcmp(Command, "save at")!= 0){
        edit(410);
    }
}

void editSup(int limit, int a ,int &lY, int &curX, char *line, int allines, int &ign, int &ignchars){
    const char *lb = line;
    char buffer[100000], B2[100000], B3[100000], Line1[100000], Line2[100000], zero[4] = {""};
    char *bp = buffer;
    Change = 1;
    Clearstring(buffer);
    int b = 0;
    clear();
    
    if(a != 127 && a != 10){
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
        
    } else if (a == 127 || a == KEY_BACKSPACE){
        if(curX == 0 && lY != 0){
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
                ignchars = (int)strlen(buffer)  - getmaxx(win) + 1;
            }
            goto end;
        } else {
            es001((curX+ignchars)-1, line);
            cpstr(line, buffer);
            scan(buffer);
            curX--;
        }
    } else if(a == 10 || a == '\n'){
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
    }
    
    scan(buffer);
    Changeline(lY+ign, buffer, true);
end:
    printw("");
}

void edit(int a){
    char ex[100000];
    static int lY = 0, lX = 0,ign = 0, lines = 0, allines, ignchars = 0;
restart:
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
            wprintw(win,"ERR");
        } else if (a == 410) {
        CLEAR:
            clear();
            resize();
            Meedit(NULL);
        } else if (a == 27) {
            //mvwprintw(win,30, 1,"line: %d | coln: %d", lY, lX);
            CommandLine(lY, lX, ign);
        } else if (a == 9) {
            a = ' ';
            for(int z = 0 ; z < 4 ; z ++){
            editSup(lines,a, lY,lX, ex, allines, ign,ignchars);
            see(false,lY, ex,allines, ign, 0);
            scan(ex);
            }
        }else {
            if(ReadOnly == false){
                
                editSup(lines,a, lY,lX, ex, allines, ign, ignchars);
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
    
    //wattron(win, COLOR_PAIR(1));
    lines = see(true,lY, ex,allines, ign, ignchars);
    //wattroff(win, COLOR_PAIR(1));
    scan(ex);
    
    if(strlen(ex) < getmaxx(win))
        ignchars = 0;
    
    if(lX > strlen(ex))
        lX = (int)strlen(ex);
    
    //mvwprintw(win,30, 1,"line: %d | coln: %d", lY,lX);
    //mvwprintw(win,30, 1,"line: %d<%d | coln: %d | IGN: %d | realine: %d | R.line %d>%d", lY,allines, lX, ign, lY+ign, lines, getmaxy(win));
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
    
    FILE *test = fopen(Path, "rt");
    if(test == NULL){
        fclose(test);
        test = fopen(Path, "wt");
        if(test == NULL){
            printf("\e[1mMeedit:\e[0m \"%s\"not found\n", Path);
            status = 1;
        }
    }
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
    Mirror(RPath, Path);
}

void end(int exitStatus){
    char Command[10000], rm[4] = {"rm "};
    SStr(rm, Path, Command);
    if(UsageMode == Normal && onhelp == false)
        system(Command);
    endwin();
    if(UsageMode == Newfile)
        delAfileSP();
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
