#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <curses.h>
#include <sys/types.h>
#include <unistd.h>

#define ENTER 10
#define key_F 70
#define key_f 102
#define key_BKSP 32

#define LOG_ERR 9, __FILE__, __LINE__
#define LOG_NML 6, __FILE__, __LINE__
#define LOG_DBG 3, __FILE__, __LINE__

int ARR_SIZE_X = 0;
int ARR_SIZE_Y = 0;
int MINE_NUM = 0;
int **glb_arr;
int **flg_arr;
WINDOW *w_frame = NULL;
WINDOW **w_grid;
int pos = 0;
int flag_num = 0;
int best_time[3];
int win_times[3];
int total_times[3];


int select_level();
int init_arr();
int disp_one_grid(int x, int y, WINDOW **w_one);
int arr2log(int **arr);
int free_game();
int iswin();
int compute_blank(int x, int y);
int game_over();
int free_win();
int press_right();
int press_left();
int press_up();
int press_down();
int disp_grid();
int gen_grid() ;
int init_game();
int Log(int fmt, ...);
char *GetTime_HHMMSS( char * buff ); 

int main()
{
	int key = 0;
	char str[3] = {0};
	int number = 0;
	int exitflag = 0;
	int isfirst = 0;
	time_t start_time, end_time;
	unsigned int elapsed_time;
	char elapsed_time_str[7] = {0};
	int flag = 0;
	char scurrtime[14+1] = {0};
	FILE *fp = NULL;

	Log(LOG_DBG, "begin new game...");
	isfirst = 1;
	init_game();

	gen_grid();
	disp_grid();

	while(1) {
		sprintf(str, "%2d Left", MINE_NUM-flag_num);
		mvwaddstr(w_frame, 11, 2, str);
		wrefresh(w_frame);

		wattron(w_grid[pos], COLOR_PAIR(1));
		box(w_grid[pos], 0, 0);
		wattroff(w_grid[pos], COLOR_PAIR(1));
		wrefresh(w_grid[pos]);

		key = wgetch(w_frame);
		switch(key) {
			case KEY_RIGHT: press_right(); break;
			case KEY_LEFT: press_left(); break;
			case KEY_UP: press_up(); break;
			case KEY_DOWN: press_down(); break;
			case key_BKSP:
						   if(flag==0) { 
							   start_time = time(NULL); 
							   GetTime_HHMMSS(scurrtime);
							   Log(LOG_DBG, "start-time : %s", scurrtime);
							   flag = 1; 
							   if(ARR_SIZE_X==9 && ARR_SIZE_Y==9 && MINE_NUM==10) total_times[0]++;
							   else if(ARR_SIZE_X==16 && ARR_SIZE_Y==16 && MINE_NUM==40) total_times[1]++;
							   else if(ARR_SIZE_X==16 && ARR_SIZE_Y==30 && MINE_NUM==99) total_times[2]++;
						   }

						   while(1) {
							   if(flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y]==2) break;
							   number = glb_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y];
							   if(number==9) {
								   if(isfirst==1) { 
									   Log(LOG_DBG, "regen array");
									   free_win();
									   gen_grid();
									   disp_grid();
									   continue;
								   }
								   wattron(w_grid[pos], COLOR_PAIR(2));
								   strcpy(str, "[]");
								   mvwaddstr(w_grid[pos], 1, 1, str);
								   wattroff(w_grid[pos], COLOR_PAIR(2));
								   wrefresh(w_grid[pos]);
								   flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y] = 1;

								   wattron(w_frame, COLOR_PAIR(1));
								   mvwaddstr(w_frame, 3, 2, "You lose!");
								   wattroff(w_frame, COLOR_PAIR(1));

								   game_over();
								   exitflag = 1;
							   } else if(number==0) {
								   Log(LOG_DBG, "compute_blank...");
								   compute_blank(pos/ARR_SIZE_Y, pos%ARR_SIZE_Y);
							   } else {
								   sprintf(str, "%d ", number);
								   mvwaddstr(w_grid[pos], 1, 1, str);
								   wrefresh(w_grid[pos]);
								   flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y] = 1;
							   }
							   break;
						   }

						   isfirst = 0;

						   if(exitflag!=1 && 1==iswin()) {
							   Log(LOG_DBG, "win!");
							   wattron(w_frame, COLOR_PAIR(2));
							   mvwaddstr(w_frame, 3, 2, "You win!");
							   wattroff(w_frame, COLOR_PAIR(2));
							   mvwaddstr(w_frame, 11, 2, " 0 Left");
							   wrefresh(w_frame);
							   game_over();
							   exitflag = 2;

							   if(ARR_SIZE_X==9 && ARR_SIZE_Y==9 && MINE_NUM==10) win_times[0]++;
							   else if(ARR_SIZE_X==16 && ARR_SIZE_Y==16 && MINE_NUM==40) win_times[1]++;
							   else if(ARR_SIZE_X==16 && ARR_SIZE_Y==30 && MINE_NUM==99) win_times[2]++;
						   }

						   break;
			case key_F:
			case key_f:
						   if(flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y]==0) {
							   wattron(w_grid[pos], A_REVERSE);
							   wattron(w_grid[pos], COLOR_PAIR(1));
							   mvwaddstr(w_grid[pos], 1, 1, "  ");
							   wattroff(w_grid[pos], COLOR_PAIR(1));
							   wattroff(w_grid[pos], A_REVERSE);
							   wrefresh(w_grid[pos]);
							   flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y] = 2;
							   flag_num++;
						   } else if(flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y]==2) {
							   wattron(w_grid[pos], COLOR_PAIR(3));
							   mvwaddstr(w_grid[pos], 1, 1, "  ");
							   wattroff(w_grid[pos], COLOR_PAIR(3));
							   wrefresh(w_grid[pos]);
							   flg_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y] = 0;
							   flag_num--;
						   }
						   break;
			default:
						   Log(LOG_DBG, "press:%d pos:%d x=%d y=%d arr=%d", key, pos, pos/ARR_SIZE_Y, pos%ARR_SIZE_Y, glb_arr[pos/ARR_SIZE_Y][pos%ARR_SIZE_Y]);
						   break;
		}
		if(exitflag==1 || exitflag==2) break;

	}

	end_time = time(NULL);
	GetTime_HHMMSS(scurrtime);
	Log(LOG_DBG, "end-time : %s", scurrtime);

	elapsed_time = difftime(start_time, end_time);
	sprintf(elapsed_time_str, "Used: %04ds", -elapsed_time);
	wattron(w_frame, COLOR_PAIR(exitflag));
	mvwaddstr(w_frame, 4, 2, elapsed_time_str);
	wattroff(w_frame, COLOR_PAIR(exitflag));


	if(exitflag==2) {
		if(ARR_SIZE_X==9 && ARR_SIZE_Y==9 && MINE_NUM==10 && -elapsed_time<best_time[0]) best_time[0] = -elapsed_time;
		else if(ARR_SIZE_X==16 && ARR_SIZE_Y==16 && MINE_NUM==40 && -elapsed_time<best_time[1]) best_time[1] = -elapsed_time;
		else if(ARR_SIZE_X==16 && ARR_SIZE_Y==30 && MINE_NUM==99 && -elapsed_time<best_time[2]) best_time[2] = -elapsed_time;
	}

	mvwaddstr(w_frame, 9, 2, "Press Enter");

	if((ARR_SIZE_X==9 && ARR_SIZE_Y==9 && MINE_NUM==10) \
			|| (ARR_SIZE_X==16 && ARR_SIZE_Y==16 && MINE_NUM==40) \
			|| (ARR_SIZE_X==16 && ARR_SIZE_Y==30 && MINE_NUM==99)) {
		fp = fopen(".MineSweeper.dat", "w");
		if (fp != (FILE*)NULL) {
			fprintf(fp, "%04d%04d%04d%04d%04d%04d%04d%04d%04d", \
					best_time[0]?best_time[0]:9999, total_times[0], win_times[0], \
					best_time[1]?best_time[1]:9999, total_times[1], win_times[1], \
					best_time[2]?best_time[2]:9999, total_times[2], win_times[2]);
			fclose(fp);
		}
	}

	wrefresh(w_frame);
	do {
		key = wgetch(w_frame);
	} while(key!=ENTER);

	free_game();
	return 0;
}

int rand_a_num(int from, int to) {
	int tmp=0;
	if(from>to) tmp=from,from=to,to=tmp;

	return (rand()%(to-from+1))+from;
}

int is_in_array(int num, int* numbers, int size) {
	int i=0;

	while(i<size) {
		if(numbers[i++]==num) return 1;
	}

	return 0;
}

int gen_grid() {
	int i = 0;
	int j = 0;
	int num = 0;
	int idx_arr[480];
	int rand_num = 0;
	Log(LOG_DBG, "gen array..");
	memset(idx_arr, 0x0, sizeof(idx_arr));

	for(i=0; i<ARR_SIZE_X; i++)
		for(j=0; j<ARR_SIZE_Y; j++)
			glb_arr[i][j] = 0;

	for(i=0; i<MINE_NUM; i++) {
		do {
			rand_num = rand_a_num(0, ARR_SIZE_X*ARR_SIZE_Y-1);
		} while(is_in_array(rand_num, idx_arr, MINE_NUM));

		idx_arr[i] = rand_num;
	}

	Log(LOG_DBG, "mine_num:%d arr_size_x:%d arr_size_y:%d", MINE_NUM, ARR_SIZE_X, ARR_SIZE_Y);
	for(i=0; i<MINE_NUM; i++) {
		glb_arr[idx_arr[i]/ARR_SIZE_Y][idx_arr[i]%ARR_SIZE_Y] = 9; 
	}

	for(i=0; i<ARR_SIZE_X; i++) {
		for(j=0; j<ARR_SIZE_Y; j++) {
			if(glb_arr[i][j]==9) continue;
			num = 0;
			if(i-1>=0 && j-1>=0) {
				if(glb_arr[i-1][j-1]==9) num++;
				if(glb_arr[i-1][j]==9) num++;
				if(glb_arr[i][j-1]==9) num++;

				if(i==ARR_SIZE_X-1 && j==ARR_SIZE_Y-1) {
				} else if(i==ARR_SIZE_X-1) {
					if(glb_arr[i-1][j+1]==9) num++;
					if(glb_arr[i][j+1]==9) num++;
				} else if(j==ARR_SIZE_Y-1) {
					if(glb_arr[i+1][j-1]==9) num++;
					if(glb_arr[i+1][j]==9) num++;
				} else {
					if(glb_arr[i+1][j-1]==9) num++;
					if(glb_arr[i+1][j]==9) num++;
					if(glb_arr[i-1][j+1]==9) num++;
					if(glb_arr[i][j+1]==9) num++;
					if(glb_arr[i+1][j+1]==9) num++;
				}
			} else if(i-1>=0) {
				if(glb_arr[i-1][j]==9) num++;
				if(glb_arr[i-1][j+1]==9) num++;
				if(glb_arr[i][j+1]==9) num++;

				if(i==ARR_SIZE_X-1) { 
				} else {
					if(glb_arr[i+1][j]==9) num++;
					if(glb_arr[i+1][j+1]==9) num++;
				}
			} else if(j-1>=0) {
				if(glb_arr[i][j-1]==9) num++;
				if(glb_arr[i+1][j-1]==9) num++;
				if(glb_arr[i+1][j]==9) num++;

				if(j==ARR_SIZE_Y-1) {
				} else {
					if(glb_arr[i][j+1]==9) num++;
					if(glb_arr[i+1][j+1]==9) num++;
				}
			} else {
				if(glb_arr[i+1][j]==9) num++;
				if(glb_arr[i][j+1]==9) num++;
				if(glb_arr[i+1][j+1]==9) num++;
			}

			glb_arr[i][j] = num;
		}
	}

	Log(LOG_DBG, "gen array: ");
	arr2log(glb_arr);

	return 0;
}

int disp_grid()
{
	int i = 0;
	int j = 0;
	WINDOW *w_tmp = NULL; 

	for(i=0; i<ARR_SIZE_X; i++) {
		for(j=0; j<ARR_SIZE_Y; j++) {       
			disp_one_grid(1+i*3, 17+j*4, &w_tmp); 
			w_grid[i*ARR_SIZE_Y+j] = w_tmp;
		}       
	}

	wrefresh(w_frame);

	return 0;
}


int disp_one_grid(int x, int y, WINDOW **w_one)
{
	WINDOW *w_tmp = NULL; 
	int flag = 0;

	w_tmp = derwin(w_frame, 3, 4, x, y);
	werase(w_tmp);

	wattron(w_tmp, COLOR_PAIR(3));
	mvwaddstr(w_tmp, 1, 1, "  ");
	wattroff(w_tmp, COLOR_PAIR(3));

	box(w_tmp, 0, 0);
	wrefresh(w_tmp);

	*w_one = w_tmp;
	return 0;
}



int init_game()
{
	Log(LOG_DBG, "init game..");

	char str[37] = {0};
	char strtmp[5] = {0};
	FILE *fp = NULL; 
	int i = 0;

	for(i=0; i<3; i++) {
		best_time[i] = 0; 
		total_times[i] = 0;
		win_times[i] = 0;
	}

	fp = fopen(".MineSweeper.dat", "r");
	if (fp != (FILE*)NULL) {
		fscanf(fp, "%s", str);
		Log(LOG_DBG, "read file. str:%s", str);

		strncpy(strtmp, str, 4); 
		best_time[0] = atoi(strtmp);
		strncpy(strtmp, str+4, 4); 
		total_times[0] = atoi(strtmp);
		strncpy(strtmp, str+8, 4); 
		win_times[0] = atoi(strtmp);

		strncpy(strtmp, str+12, 4); 
		best_time[1] = atoi(strtmp);
		strncpy(strtmp, str+16, 4); 
		total_times[1] = atoi(strtmp);
		strncpy(strtmp, str+20, 4); 
		win_times[1] = atoi(strtmp);

		strncpy(strtmp, str+24, 4); 
		best_time[2] = atoi(strtmp);
		strncpy(strtmp, str+28, 4); 
		total_times[2] = atoi(strtmp);
		strncpy(strtmp, str+32, 4); 
		win_times[2] = atoi(strtmp);
		fclose(fp);
	}

	Log(LOG_DBG, "mine_num:%d arr_size_x:%d arr_size_y:%d", MINE_NUM, ARR_SIZE_X, ARR_SIZE_Y);
	srand(time(NULL));
	initscr();
	curs_set(0);
	noecho();
	refresh();

	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_BLACK, COLOR_CYAN);

	init_arr();

	w_frame = newwin(ARR_SIZE_X*3+2, ARR_SIZE_Y*4+2+16, (LINES-ARR_SIZE_X*3)/2, (COLS-ARR_SIZE_Y*4)/2-16);
	keypad(w_frame, TRUE);
	mvwaddstr(w_frame, 2, 2, "MineSweeper");
	mvwaddstr(w_frame, 6, 2, "F/f   :flag");
	mvwaddstr(w_frame, 7, 2, "Space :open");
	mvwaddstr(w_frame, 8, 2, "UDLR  :move");
	wrefresh(w_frame);

	mvwaddstr(w_frame, 13, 2, "Std Simple");
	if(best_time[0]==9999) {
		mvwaddstr(w_frame, 14, 2, "Best : ----");
	} else {
		sprintf(str, "Best : %04d", best_time[0]);
		mvwaddstr(w_frame, 14, 2, str);
	}
	sprintf(str, "Total: %04d", total_times[0]);
	mvwaddstr(w_frame, 15, 2, str);
	sprintf(str, "Wins : %04d", win_times[0]);
	mvwaddstr(w_frame, 16, 2, str);

	mvwaddstr(w_frame, 18, 2, "Std Medium");
	if(best_time[1]==9999) {
		mvwaddstr(w_frame, 19, 2, "Best : ----");
	} else {
		sprintf(str, "Best : %04d", best_time[1]);
		mvwaddstr(w_frame, 19, 2, str);
	}
	sprintf(str, "Total: %04d", total_times[1]);
	mvwaddstr(w_frame, 20, 2, str);
	sprintf(str, "Wins : %04d", win_times[1]);
	mvwaddstr(w_frame, 21, 2, str);

	mvwaddstr(w_frame, 23, 2, "StdDifficult");
	if(best_time[2]==9999) {
		mvwaddstr(w_frame, 24, 2, "Best : ----");
	} else {
		sprintf(str, "Best : %04d", best_time[2]);
		mvwaddstr(w_frame, 24, 2, str);
	}
	sprintf(str, "Total: %04d", total_times[2]);
	mvwaddstr(w_frame, 25, 2, str);
	sprintf(str, "Wins : %04d", win_times[2]);
	mvwaddstr(w_frame, 26, 2, str);

	return 0;
}


int free_game()
{
	Log(LOG_DBG, "free...");
	free_win();
	delwin(w_frame);
	endwin();
	free(glb_arr);
	free(flg_arr);

	return 0;
}

int free_win()
{
	int i = 0;

	for(i=0; i<ARR_SIZE_X*ARR_SIZE_Y; i++) {
		delwin(w_grid[i]); 
	}

	return 0;
}

int Log(int fmt, ...)
{
	va_list   args;
	int       nLevel;
	int       nSysLevel;
	char      *file;
	char      *fmt1;
	FILE      *fp;
	int       line;
	char      sLogFile[128 + 1];
	char      sCurrTime[14 + 1];
	char      sMessage[1024 + 1];

	memset(sCurrTime, 0x00, sizeof(sCurrTime));
	memset(sMessage, 0x00, sizeof(sMessage));

	nLevel = fmt;

	va_start(args, fmt);
	file = va_arg(args, char *);
	line = va_arg(args, int);
	fmt1 = va_arg(args, char *);
	vsprintf(sMessage, fmt1, args);
	va_end(args);


	if (getenv("TRACELEVEL") == NULL) nSysLevel = 0;
	else nSysLevel = atoi(getenv("TRACELEVEL"));
	if (nLevel < nSysLevel) return 0;

	memset(sLogFile, 0, sizeof(sLogFile));
	strcpy(sLogFile, "MineSweeper.log");

	fp = fopen(sLogFile, "a+");
	if (fp != (FILE*)NULL) {
		GetTime_HHMMSS(sCurrTime);
		fprintf(fp, "[%s][%06d][% 5s:%05d] %s\n", sCurrTime, getpid(), file, line, sMessage);
		fclose(fp);
		return 0;
	}

	return 0;
}


char *GetTime_HHMMSS( char * buff )
{
	time_t time(), now;
	struct tm * tTime, * localtime();

	time( &now );
	tTime = localtime( &now );

	sprintf( buff, "%.2d%.2d%.2d", tTime->tm_hour,
			tTime->tm_min, tTime->tm_sec );
	return buff;
}

int arr2log(int **arr)
{
	Log(LOG_DBG, "arr2log...");
	int i = 0;
	int j = 0;
	char str[61] = {0};

	for(i=0; i<ARR_SIZE_X; i++) {
		memset(str, 0x0, sizeof(str));
		for(j=0; j<ARR_SIZE_Y; j++) {
			sprintf(str, "%s %d", str, arr[i][j]);
		}
		Log(LOG_DBG, "%s", str);
	}

	return 0;
}

int compute_blank(int x, int y)
{
	int number = 0;
	char str[3] = {0};

	werase(w_grid[ARR_SIZE_Y*x+y]);

	number = glb_arr[x][y];
	if(number==9) strcpy(str, "[]");
	else if(number==0) strcpy(str, "  ");
	else sprintf(str, "%d ", number);

	if(number==9) wattron(w_grid[pos], COLOR_PAIR(2));
	mvwaddstr(w_grid[ARR_SIZE_Y*x+y], 1, 1, str);
	if(number==9) wattroff(w_grid[pos], COLOR_PAIR(2));

	box(w_grid[ARR_SIZE_Y*x+y], 0, 0);
	wrefresh(w_grid[ARR_SIZE_Y*x+y]);
	flg_arr[x][y] = 1;

	if(glb_arr[x][y]==0) {
		if(x-1>=0&&y-1>=0 && flg_arr[x-1][y-1]==0) compute_blank(x-1, y-1);
		if(x-1>=0&&flg_arr[x-1][y]==0) compute_blank(x-1, y);    
		if(y-1>=0&&flg_arr[x][y-1]==0) compute_blank(x, y-1);    

		if(x+1<ARR_SIZE_X&&y+1<ARR_SIZE_Y&&flg_arr[x+1][y+1]==0) compute_blank(x+1, y+1);
		if(x+1<ARR_SIZE_X&&flg_arr[x+1][y]==0) compute_blank(x+1, y);
		if(y+1<ARR_SIZE_Y&&flg_arr[x][y+1]==0) compute_blank(x, y+1);

		if(x-1>=0&&y+1<ARR_SIZE_Y&&flg_arr[x-1][y+1]==0) compute_blank(x-1, y+1);
		if(x+1<ARR_SIZE_X&&y-1>=0&&flg_arr[x+1][y-1]==0) compute_blank(x+1, y-1);
	}
	return 0;
}

int press_right()
{
	box(w_grid[pos], 0, 0);
	wrefresh(w_grid[pos]);

	if((pos+1)%ARR_SIZE_Y==0) pos = pos+1-ARR_SIZE_Y;
	else pos = pos+1;

	wattron(w_grid[pos], COLOR_PAIR(1));
	box(w_grid[pos], 0, 0);
	wattroff(w_grid[pos], COLOR_PAIR(1));
	wrefresh(w_grid[pos]);

	return 0;
}

int press_left()
{
	box(w_grid[pos], 0, 0);
	wrefresh(w_grid[pos]);

	if((pos+1)%ARR_SIZE_Y==1) pos = pos-1+ARR_SIZE_Y;
	else pos = pos-1;

	wattron(w_grid[pos], COLOR_PAIR(1));
	box(w_grid[pos], 0, 0);
	wattroff(w_grid[pos], COLOR_PAIR(1));
	wrefresh(w_grid[pos]);

	return 0;
}

int press_up()
{
	box(w_grid[pos], 0, 0);
	wrefresh(w_grid[pos]);

	if(pos<ARR_SIZE_Y) pos = (ARR_SIZE_X-1)*ARR_SIZE_Y+pos;
	else pos = pos - ARR_SIZE_Y;

	wattron(w_grid[pos], COLOR_PAIR(1));
	box(w_grid[pos], 0, 0);
	wattroff(w_grid[pos], COLOR_PAIR(1));
	wrefresh(w_grid[pos]);

	return 0;
}

int press_down()
{
	box(w_grid[pos], 0, 0);
	wrefresh(w_grid[pos]);

	if(pos>=(ARR_SIZE_X-1)*ARR_SIZE_Y) pos = pos-(ARR_SIZE_X-1)*ARR_SIZE_Y;
	else pos = pos+ARR_SIZE_Y;

	wattron(w_grid[pos], COLOR_PAIR(1));
	box(w_grid[pos], 0, 0);
	wattroff(w_grid[pos], COLOR_PAIR(1));
	wrefresh(w_grid[pos]);


	return 0;
}

int game_over()
{
	int i = 0;
	int j = 0;

	for(i=0; i<ARR_SIZE_X; i++) {
		for(j=0; j<ARR_SIZE_Y; j++) {
			if(glb_arr[i][j]==9) {
				if(flg_arr[i][j]==2) wattron(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(2));
				else wattron(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(1));
				mvwaddstr(w_grid[i*ARR_SIZE_Y+j], 1, 1, "[]");
				if(flg_arr[i][j]==2) wattroff(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(2));
				else wattroff(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(1));
				wrefresh(w_grid[i*ARR_SIZE_Y+j]);
			} else if(flg_arr[i][j]==2) {
				wattron(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(1));
				mvwaddstr(w_grid[i*ARR_SIZE_Y+j], 1, 1, "XX");
				wattroff(w_grid[i*ARR_SIZE_Y+j], COLOR_PAIR(1));
				wrefresh(w_grid[i*ARR_SIZE_Y+j]);
			}
		}
	}
	return 0;
}

int iswin()
{
	int ret = 0;
	int i = 0;
	int j = 0;
	int open_num = 0;

	for(i=0; i<ARR_SIZE_X; i++) {
		for(j=0; j<ARR_SIZE_Y; j++) {
			if(flg_arr[i][j]==1) open_num++;
		}
	}

	if(open_num==ARR_SIZE_X*ARR_SIZE_Y-MINE_NUM) ret = 1;

	return ret;
}

int init_arr()
{
	int sel_typ = 0;
	int i = 0;
	int j = 0;

	sel_typ = select_level();
	Log(LOG_DBG, "sel_typ:%d mine_num:%d", sel_typ, MINE_NUM);

	if(sel_typ==0) { ARR_SIZE_X = 9; ARR_SIZE_Y = 9;}
	else if(sel_typ==1) { ARR_SIZE_X = 16; ARR_SIZE_Y = 16;}
	else { ARR_SIZE_X = 16; ARR_SIZE_Y = 30;}

	glb_arr = (int**)malloc(ARR_SIZE_X*sizeof(int*));
	flg_arr = (int**)malloc(ARR_SIZE_X*sizeof(int*));
	for(i=0; i<ARR_SIZE_X; i++) {
		glb_arr[i] = (int*)malloc(ARR_SIZE_Y*sizeof(int));
		flg_arr[i] = (int*)malloc(ARR_SIZE_Y*sizeof(int));
		for(j=0; j<ARR_SIZE_Y; j++) {
			glb_arr[i][j] = 0;
			flg_arr[i][j] = 0;
		}
	}

	w_grid = (WINDOW**)malloc(ARR_SIZE_X*ARR_SIZE_Y*sizeof(WINDOW*));
	return 0;
}


int select_level()
{
	WINDOW *w_tmp = NULL;
	int key = 0;
	int select = 0;
	int mine_num1 = 10;
	int mine_num2 = 40;
	int mine_num3 = 99;
	char str[30] = {0};

	w_tmp = newwin(15, 41, LINES/2-10, COLS/2-20);
	keypad(w_tmp, TRUE);
	box(w_tmp, 0, 0);
	mvwaddstr(w_tmp, 0, 4, "Select  Level");
	mvwaddstr(w_tmp, 12, 3, "[Up/Down]   [Left/Right]    [Enter]");
	wrefresh(w_tmp);

	while(1) {
		if(select==0)wattron(w_tmp, A_REVERSE);
		sprintf(str, "   Simple: 09X09 %3dMine", mine_num1);
		mvwaddstr(w_tmp, 4, 20-12, str);
		if(select==0)wattroff(w_tmp, A_REVERSE);

		if(select==1)wattron(w_tmp, A_REVERSE);
		sprintf(str, "   Medium: 16X16 %3dMine", mine_num2);
		mvwaddstr(w_tmp, 6, 20-12, str);
		if(select==1)wattroff(w_tmp, A_REVERSE);

		if(select==2)wattron(w_tmp, A_REVERSE);
		sprintf(str, "Difficult: 16X30 %3dMine", mine_num3);
		mvwaddstr(w_tmp, 8, 20-12, str);
		if(select==2)wattroff(w_tmp, A_REVERSE);

		do{
			key = wgetch(w_tmp); 
		} while(key!=KEY_UP && key!=KEY_DOWN && key!=ENTER && key!=KEY_LEFT && key!=KEY_RIGHT);

		if(key==KEY_UP) {
			if(select==0) select = 2;
			else select--;

			mine_num1 = 10;
			mine_num2 = 40;
			mine_num3 = 99;
		} else if(key==KEY_DOWN) {
			if(select==2) select = 0;
			else select++;

			mine_num1 = 10;
			mine_num2 = 40;
			mine_num3 = 99;
		} else if(key==KEY_LEFT) {
			if(select==0) {
				if(mine_num1>10) mine_num1--; 
			} else if(select==1) {
				if(mine_num2>10) mine_num2--;
			} else {
				if(mine_num3>10) mine_num3--; 
			}
		} else if(key==KEY_RIGHT) {
			if(select==0) {
				if(mine_num1<67) mine_num1++; 
			} else if(select==1) {
				if(mine_num2<232) mine_num2++;
			} else {
				if(mine_num3<442) mine_num3++; 
			}
		} else {
			Log(LOG_DBG, "your choose：%s", select==0?"Simple":(select==1?"Medium":"Difficult"));
			break;
		}
	}

	MINE_NUM = select==0?mine_num1:(select==1?mine_num2:mine_num3);

	werase(w_tmp);
	wrefresh(w_tmp);
	delwin(w_tmp);
	return select;
}
