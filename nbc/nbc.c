#include <curses.h>
#include <panel.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STRING_MAX 134217729 
#define OBASE_MAX 134217729
#define IBASE_MAX 16
#define SCALE_MAX 134217729

char bccmd[STRING_MAX];
WINDOW *help, *calc, *settings, *var;
PANEL *panhelp, *pancalc, *pansettings, *panvar;

void quit()
{
	del_panel(panhelp);
	del_panel(pancalc);
	del_panel(pansettings);
	delwin(help);
	delwin(calc);
	delwin(settings);
	endwin();
}

int main(void)
{
	FILE *bc_pipe, *bcinput, *bc_err;
	char *output;
	int i, j, c, parenthesis, x, y, xmax, ymax; 
	int top=2, nbytes = 100, bytes_read, scale=50, ibase=10, obase=10;
	int xparenthesis[1024];
	bool readch;

	initscr();
	atexit(quit);
	start_color();
	curs_set(1);
	cbreak();
	noecho();

	init_pair(0, COLOR_WHITE, COLOR_BLACK);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	init_pair(4, COLOR_GREEN, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
	init_pair(6, COLOR_BLUE, COLOR_BLACK);
	init_pair(7, COLOR_GREEN, COLOR_RED);
	init_pair(8, COLOR_CYAN, COLOR_MAGENTA);
	init_pair(9, COLOR_BLUE, COLOR_YELLOW);
	init_pair(10, COLOR_RED, COLOR_GREEN);
	init_pair(11, COLOR_MAGENTA, COLOR_CYAN);
	init_pair(12, COLOR_YELLOW, COLOR_BLUE);
	init_pair(13, COLOR_CYAN, COLOR_BLUE);

	getmaxyx(stdscr, ymax, xmax);
	calc=newwin(ymax, xmax, 0, 0);
	pancalc=new_panel(calc);
	keypad(calc, TRUE);
	scrollok(calc, TRUE);

	help=newwin(ymax, xmax, 0, 0);
	panhelp=new_panel(help);
	wattrset(help, COLOR_PAIR(6) | A_BOLD);
	mvwaddstr(help, 0, 0, "Hilfe");
	wattrset(help, COLOR_PAIR(0));
	i=0;
	while (i < xmax) mvwaddch(help, 1, i++, ACS_HLINE);

	settings=newwin(ymax, xmax, 0, 0);
	pansettings=new_panel(settings);
	wattrset(settings, COLOR_PAIR(6) | A_BOLD);
	mvwaddstr(settings, 0, 0, "Einstellungen");
	wattrset(settings, COLOR_PAIR(2));
	mvwaddch(settings, 3, 1, 's');
	mvwaddch(settings, 4, 1, 'i');
	mvwaddch(settings, 5, 1, 'o');
	wattrset(settings, COLOR_PAIR(5));
	mvwprintw(settings, 3, 2, "cale = %u", scale);
	mvwprintw(settings, 4, 2, "base = %u", ibase);
	mvwprintw(settings, 5, 2, "base = %u", obase);
	wattrset(settings, COLOR_PAIR(0));
	i=0;
	while (i < xmax) mvwaddch(settings, 1, i++, ACS_HLINE);

	while (TRUE) {
		readch=TRUE;
		i=0;
		bccmd[0]='\0';
		parenthesis=0;
		while (readch) {
			switch(c=wgetch(calc)) {
				case '\n':
					if (top != 2) break;
					readch=FALSE;
					break;
				case KEY_LEFT:
					if (top != 2) break;
					if(i > 0) {
						getyx(calc, y, x);
						wmove(calc, y, x-1);
						--i;
					}
					break;
				case KEY_RIGHT:
					if (top !=2) break;
					if (bccmd[i] != '\0') {
						getyx(calc, y, x);
						wmove(calc, y, x+1);
						++i;
					}
					break;
				case KEY_F(1):
					curs_set(0);
					top_panel(panhelp);
					update_panels();
					doupdate();
					top=1;
					break;
				case KEY_F(2):
					curs_set(1);
					top_panel(pancalc);
					update_panels();
					doupdate();
					top=2;
					break;
				case KEY_F(3):
					curs_set(0);
					top_panel(panvar);
					update_panels();
					doupdate();
					top=3;
					break;
				case KEY_F(4):
					curs_set(0);
					top_panel(pansettings);
					update_panels();
					doupdate();
					top=4;
					break;
				case KEY_F(10):
					return 0;
					break;
				case KEY_BACKSPACE:
					if (top !=2) break;
					if (i > 0) {
						getyx(calc, y, x);
						mvwaddch(calc, y, --i, ' ');
						wmove(calc, y, i);
						bccmd[i]=' ';
						j=i;
						while(bccmd[j++] != '\0') {
							bccmd[j-1]=bccmd[j];
							if (bccmd[j] != '\0') {
								mvwaddch(calc, y, j-1, bccmd[j]);
							} else {
								mvwaddch(calc, y, j-1, ' ');
							}
						}
						wmove(calc, y, i);
					}
					break;
				default:
					if (top == 4 && c == 's') {
						curs_set(1);
						mvwaddstr(settings, 3, 9, "                    ");
						wmove(settings, 3, 9);
						echo();
						wscanw(settings, "%9u", &scale);
						curs_set(0);
						noecho();
						if (scale > SCALE_MAX) scale=SCALE_MAX;
						if (scale < 1) scale=1;
						wattrset(settings, COLOR_PAIR(5));
						mvwaddstr(settings, 3, 9, "                    ");
						mvwprintw(settings, 3, 9, "%u", scale);
						update_panels();
						doupdate();
					}

					if (top == 4 && c == 'i') {
						curs_set(1);
						mvwaddstr(settings, 4, 9, "                    ");
						wmove(settings, 4, 9);
						echo();
						wscanw(settings, "%9u", &ibase);
						curs_set(0);
						noecho();
						if (ibase > IBASE_MAX) ibase=IBASE_MAX;
						if (ibase < 1) ibase=1;
						wattrset(settings, COLOR_PAIR(5));
						mvwaddstr(settings, 4, 9, "                    ");
						mvwprintw(settings, 4, 9, "%u", ibase);
						update_panels();
						doupdate();
					}

					if (top == 4 && c == 'o') {
						curs_set(1);
						mvwaddstr(settings, 5, 9, "                    ");
						wmove(settings, 5, 9);
						echo();
						wscanw(settings, "%9u", &obase);
						curs_set(0);
						noecho();
						if (obase > OBASE_MAX) obase=OBASE_MAX;
						if (obase < 2) obase=2;
						wattrset(settings, COLOR_PAIR(5));
						mvwaddstr(settings, 5, 9, "                    ");
						mvwprintw(settings, 5, 9, "%u", obase);
						update_panels();
						doupdate();
					}

					if (top !=2) break;
					j=i;
					while (bccmd[j] != '\0') j++;
					bccmd[j+1]='\0';
					while (j > i) {
						bccmd[j]=bccmd[--j];
					}
					getyx(calc, y, x);
					switch(c) {
						case ('+'):
						case ('-'):
						case ('*'):
						case ('/'):
						case ('%'):
						case ('^'):
							wattrset(calc, COLOR_PAIR(0));
							break;
						case ('0'):
						case ('1'):
						case ('2'):
						case ('3'):
						case ('4'):
						case ('5'):
						case ('6'):
						case ('7'):
						case ('8'):
						case ('9'):
						case ('.'):
							wattrset(calc, COLOR_PAIR(6) | A_BOLD);
							break;
						case ('('):
							wattrset(calc, COLOR_PAIR(parenthesis%6+7));
							xparenthesis[parenthesis++]=i;
							break;
						case (')'):
							if (bccmd[xparenthesis[--parenthesis]] == '(') {
								wattrset(calc, COLOR_PAIR(parenthesis%6+1));
								mvwaddch(calc, y, xparenthesis[parenthesis], '(');
							} 
							else {
								wattrset(calc, COLOR_PAIR((parenthesis+1)%6+7));
							}
							break;
						default:
							wattrset(calc, COLOR_PAIR(1) | A_BOLD);
					}
					mvwinsch(calc, y, i++, c);
					wmove(calc, y, i);
					bccmd[i-1]=(char) c;
			}
		}

		if (!strcmp(bccmd, "quit")) return 0;
		if ((bcinput=fopen("/tmp/nbc.bc", "w")) == NULL) {
			fprintf(stderr, "I can't open \"/tmp/nbc.bc\".");
			return EXIT_FAILURE;
		}
		fprintf(bcinput, "scale=%i\nobase=%i\nibase=%i\n", scale, obase, ibase);
		fprintf(bcinput, strcat(bccmd, "\n"));
		fprintf(bcinput, "quit\n");
		fclose(bcinput);

		getmaxyx(calc, ymax, xmax);
		bc_pipe = popen("bc -lq /tmp/nbc.bc 2>/tmp/nbc.err", "r");
		if(!bc_pipe)
		{
			fprintf(stderr, "Pipe failed.\n");
			return EXIT_FAILURE;
		}

		output = (char *) malloc(nbytes+1);
		bytes_read = getdelim(&output, &nbytes, "\n\n", bc_pipe);

		if(pclose(bc_pipe) != 0)
		{
			fprintf(stderr, "Could not run 'echo' or 'bc'.\n");
		}

		getyx(calc, y, x);
		getmaxyx(calc, ymax, xmax);
		if (y >= ymax-1) {
			scroll(calc);
			wmove(calc, --y, 0);
		}
		wattrset(calc, COLOR_PAIR(5) | A_BOLD);
		mvwaddstr(calc, ++y, 0, output);
		wrefresh(calc);
		free(output);

		getyx(calc, y, x);
		getmaxyx(calc, ymax, xmax);
		if (y >= ymax-1) {
			scroll(calc);
			wmove(calc, --y, 0);
		}
		if ((bc_err=fopen("/tmp/nbc.err", "r")) == NULL) {
			fprintf(stderr, "I can't open \"/tmp/nbc.err\".");
			return EXIT_FAILURE;
		}
		else {
			x=0;
			while ((c=getc(bc_err))!=EOF) {
				if (c == '\n') c=' ';
				wattrset(calc, COLOR_PAIR(0));
				mvwinsch(calc, ymax-2, x, ' ');
				wattrset(calc, COLOR_PAIR(13));
				mvwinsch(calc, ymax-1, x++, c);
			}
			while (x < xmax) {
				wattrset(calc, COLOR_PAIR(13));
				mvwinsch(calc, ymax-1, x, ' ');
				wattrset(calc, COLOR_PAIR(0));
				mvwinsch(calc, ymax-2, x++, ' ');
			}
		}
		wmove(calc, y, 0);
	}
	return 0;
}
