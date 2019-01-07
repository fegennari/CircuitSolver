#include "CircuitSolver.h"
#include "Draw.h"


// Circuit Solver is copyright © 1998-2001 by Frank Gennari.
// Circuit Solver, C.S.Games.cpp, or any other Circuit Solver source files
// may not be redistributed, copied, or modified in any way.


// Circuit Solver Games
// By Frank Gennari
extern int graph_enable, x_limit, y_limit, o_size;
extern box num;



struct high_score {

	char name[MAX_INPUT_CHARACTERS];
	short squares, mines;
	int time; 
	float ratio;
};





void Minesweeper();
void Play_Minesweeper();
void create_minefield();
void plant_mines(short *minefield);
void evaluate_minefield(short *minefield);
int find_adjacent(short *minefield, int location);
void print_minecount(short *minefield, int choice);
void show_safe_area(short *minefield, Boolean *uncovered, int location);
int count_unknown(short *minefield, Boolean *uncovered);
int count_incorrect(short *minefield);
void reset_signs(short *minefield);
void show_mines(short *minefield);
void explosion(int choice);
void high_scores(int time);
void show_high_scores(high_score *score, int i);
int read_high_scores(high_score *score);
void write_high_scores(char *winners_name, int mines, int time, high_score *score, int i);
void sort(high_score *score, int i);
void swap(high_score &score1, high_score &score2);


// Minesweeper

/*  0 = unknown
	1 to 8 = minecount
	9 = safe (minecount = 0)
	10 = mine
	11 = marked mine
	12 >= marked incorrect (minecount = incorrect - 15)
*/
void Minesweeper() {

	cout << "\n\n			MINESWEEPER";
	
	do {
		Play_Minesweeper();
		cout << "\nWould you like to play again?  ";
	
	} while (decision());
}





void Play_Minesweeper() {
	
	short minefield[default_num_buttons + 100] = {0};
	Boolean uncovered[default_num_buttons + 100] = {false};
	int choice, num_mines_left, incorrect, unknown, time = 0, time2, lose = 0;
	
	create_minefield();
	
	num_mines_left = num.h_boxes*num.v_boxes/5;
	
	randomize();
	
	plant_mines(minefield);
	
	evaluate_minefield(minefield);
	
	do {
		time2 = get_time();
	
		if (time == 1 || time == 0) {
			if (time == 1)
				time = get_time();
			time2 = time;
		}	
		cout << "\nClick on a block(command key to mark). " << num_mines_left << " mine";
		if (num_mines_left != 1)
			cout << "s";
		cout << " remaining.";
		
		cout << " Time: " << (time2 - time) << " second";
		if ((time2 - time) != 1)
			cout << "s";
		cout << endl;
				
		choice = which_box(GetClick());
			
		if (uncovered[choice] == true);
			
		else if (get_event()) {
			if (minefield[choice] < 12)
				--num_mines_left;
			if (minefield[choice] == 10)
				minefield[choice] = 11;
			else if (minefield[choice] < 11)
				minefield[choice] += 15;
			draw_char('M', choice, 1, (int)400/sqrt(num.h_boxes*num.v_boxes));
			draw_grid(10);	
		}
		else if (minefield[choice] == 9)	{	
			color_box(choice, 9);
			show_safe_area(minefield, uncovered, choice);
			draw_grid(10);
			reset_signs(minefield);
		}
		else if (minefield[choice] == 10) {
			color_box(choice, 9);
			show_mines(minefield);
			explosion(choice);
			draw_BULLSEYE(choice, 1);
			draw_grid(10);
			cout << "\nSorry, You Lose!" << endl;
			lose = 1;
		}
		else if (minefield[choice] >= 11 && minefield[choice] < 25) {
			color_box(choice, 14);
			draw_char('?', choice, 10, (int)400/sqrt(num.h_boxes*num.v_boxes));
			draw_grid(10);
			cout << "\nUnmark Mine?  ";
			if (decision()) {
				if (minefield[choice] >= 12 && minefield[choice] < 25)
					minefield[choice] -= 15;
				else 
					minefield[choice] = 10;
				color_box(choice, 14);
				++num_mines_left;
			}
			else {
				color_box(choice, 14);
				draw_char('M', choice, 1, (int)400/sqrt(num.h_boxes*num.v_boxes));
			}	
			draw_grid(10);
		}	
		else if (minefield[choice] != 0) {
			color_box(choice, 9);
			print_minecount(minefield, choice);
			draw_grid(10);
			uncovered[choice] = true;
		}
		incorrect = count_incorrect(minefield);
		unknown = count_unknown(minefield, uncovered);
		
		if (time == 0)
			time = 1;
	
	} while (!lose && !(num_mines_left == 0 && incorrect == 0 && unknown == 0));
	
	if (num_mines_left == 0 && incorrect == 0 && unknown == 0) {
		time2 = get_time();
		cout << "\nCongratulations, You WIN!!!   Time: " << (time2 - time) << " seconds ("
		     << (time2 - time)/(num.h_boxes*num.v_boxes/5) << " seconds/mine)" << endl;
		     
		high_scores(time2 - time);
	}
}





void create_minefield() {
	
	int blocks;
	
	cout << "\n\nEnter the approximate number of blocks(" << default_num_buttons/20 << " - " << default_num_buttons << "):  ";
	blocks = in();
	
	if (blocks > default_num_buttons)
		blocks = default_num_buttons;
		
	if (blocks < default_num_buttons/10)
		blocks = default_num_buttons/20;
	
	if (!graph_enable) {
	
		Window_setup();
		
		graph_enable = 1;
	}
	whiteout();

	init_menu(blocks, 10);
	
	SetColor(14);
	RectangleDraw(0, 0, x_limit, y_limit);
	
	draw_grid(10);
}





void plant_mines(short *minefield) {
	
	int location;
	
	for (int i = 1; i <= num.h_boxes*num.v_boxes/5; ++i) {
		do {
			location = random_box();
		
		} while (location == 0 || location == (num.h_boxes - 1) || location == (num.h_boxes*(num.v_boxes - 1)) || location == (num.h_boxes*num.v_boxes - 1) || minefield[location] == 10);
		
		minefield[location] = 10;
	}
}





void evaluate_minefield(short *minefield) {

	for (int i = 0; i < num.h_boxes*num.v_boxes; ++i) {
	
		minefield[i] = find_adjacent(minefield, i);
	}
}





int find_adjacent(short *minefield, int location) {

	int count = 0, h = num.h_boxes;
	
	if (minefield[location] == 10)
		return 10;
			
	if (minefield[location] == 9)
		return 0;
		
	if (minefield[location] != 0)
		return -1;
			
	if (minefield[location + 1] == 10 && (location + 1)%h != 0)
		++count;
		
	if (minefield[location - 1] == 10 && (location + 1)%h != 1 && location > 0)
		++count;
		
	if (minefield[location + h] == 10)
		++count;
		
	if (minefield[location - h] == 10 && location > h)
		++count;
		
	if (minefield[location + h + 1] == 10 && (location + 1)%h != 0)
		++count;
		
	if (minefield[location + h - 1] == 10 && (location + 1)%h != 1 && location > 0)
		++count;
		
	if (minefield[location - h + 1] == 10 && (location + 1)%h != 0 && location >= h)
		++count;
		
	if (minefield[location - h - 1] == 10 && (location + 1)%h != 1 && location > (h + 1))
		++count;
		
	if (count == 0)
		return 9;
	
	return count;
}





void print_minecount(short *minefield, int choice) {
		
	if (minefield[choice] == 2)
		draw_char(('0' + minefield[choice]), choice, 3, (int)400/sqrt(num.h_boxes*num.v_boxes));
	else if (minefield[choice] == 3)
		draw_char(('0' + minefield[choice]), choice, 2, (int)400/sqrt(num.h_boxes*num.v_boxes));
	else if (minefield[choice] == 4)
		draw_char(('0' + minefield[choice]), choice, 12, (int)400/sqrt(num.h_boxes*num.v_boxes));
	else
		draw_char(('0' + minefield[choice]), choice, minefield[choice], (int)400/sqrt(num.h_boxes*num.v_boxes));
}





void show_safe_area(short *minefield, Boolean *uncovered, int location) {
	
	int  h = num.h_boxes;
	
	if (minefield[location] < 0)
		return;
	
	if (minefield[location] <= 11) {
		color_box(location, 9);
		uncovered[location] = true;
	}
	if (minefield[location] == 9) { 
	
		minefield[location] = -minefield[location];
		
		if ((location + 1)%h != 0)
			show_safe_area(minefield, uncovered, location + 1);
		if ((location + 1)%h != 1 && location > 0)
			show_safe_area(minefield, uncovered, location - 1);
		if (1)
			show_safe_area(minefield, uncovered, location + h);
		if (location > h)
			show_safe_area(minefield, uncovered, location - h);
		if ((location + 1)%h != 0)
			show_safe_area(minefield, uncovered, location + h + 1);
		if ((location + 1)%h != 1 && location > 0)
			show_safe_area(minefield, uncovered, location + h - 1);
		if ((location + 1)%h != 0 && location > h)
			show_safe_area(minefield, uncovered, location - h + 1);
		if ((location + 1)%h != 1 && location > (h + 1))
			show_safe_area(minefield, uncovered, location - h - 1);
	}
	if (minefield[location] > 0 && minefield[location] < 9) 
			print_minecount(minefield, location);
}





int count_unknown(short *minefield, Boolean *uncovered) {

	int unknown = 0;
	
	for (int i = 0; i < num.h_boxes*num.v_boxes; ++i)
		if ((!(minefield[i] >= 11 && minefield[i] < 25) && uncovered[i] == false) || minefield[i] == 10)
			++unknown; 
	
	return unknown;
}





int count_incorrect(short *minefield) {
	
	int incorrect = 0;
	
	for (int i = 0; i < num.h_boxes*num.v_boxes; ++i)
		if (minefield[i] >= 12 && minefield[i] < 25)
			++incorrect;
			
	return incorrect;
}





void reset_signs(short *minefield) {

	for (int i = 0; i < num.h_boxes*num.v_boxes; ++i) 
		if (minefield[i] < 0)
			minefield[i] = -minefield[i];		
}





void show_mines(short *minefield) {
	
	whiteout();
	
	draw_grid(10);
	
	for (int i = 0; i < num.h_boxes*num.v_boxes; ++i) {
		if (minefield[i] == 10) 
			draw_BULLSEYE(i, 10);
				
		else if (minefield[i] != 0 && minefield[i] < 9) 
			print_minecount(minefield, i);

		else if (minefield[i] == 11 || minefield[i] >= 12 && minefield[i] < 25) {
			if (minefield[i] >= 12) {
				draw_char('M', i, 1, (int)400/sqrt(num.h_boxes*num.v_boxes));
				draw_char('X', i, 10, (int)400/sqrt(num.h_boxes*num.v_boxes));
			}
			else
				draw_BULLSEYE(i, 3);
		}
	}
}





void explosion(int choice) {
	
	int temp = o_size, temp2;
	
	for (int i = 0; i < 5; ++i) {
		o_size = o_size/1.75;
		for (int j = 0; j < 5; ++j) {
			o_size = o_size + 13/pow((num.h_boxes*num.v_boxes), .32);
			temp2 = o_size;
			draw_BULLSEYE(choice, 1);
			delay(.1);
			o_size = temp;
			color_box(choice, 9);
			draw_grid(10);
			o_size = temp2;
		}
		o_size = temp;
	}
}





void high_scores(int time) {

	int mines = num.h_boxes*num.v_boxes/5, i;
	char winners_name[MAX_INPUT_CHARACTERS];
	high_score score[10];
	
	i = read_high_scores(score);
	
	if ((time/(num.h_boxes*num.v_boxes/5)) <= score[i].ratio || i < 9) {
		cout << "\nCongratulations, you have made a high score! Enter your name(no spaces):  ";
		strcpy(winners_name, xin(MAX_INPUT_CHARACTERS));
		write_high_scores(winners_name, mines, time, score, i);
	}
	else
		cout << "\nSorry, you did not make a high score." << endl;
}





void show_high_scores(high_score *score, int i) {

	cout << "\n\n                HIGH SCORES" << endl << endl
	     << "   NAME     SQUARES  MINES  TIME  RATIO(seconds/mine)" << endl
	     << "_____________________________________________________" << endl;
	     
	 for (int j = 0; j <= i && j < 10; ++j)    
	    cout << j + 1 << ". " << score[j].name << "      " << score[j].squares << "      " << score[j].mines << "      " << score[j].time << "      " << score[j].ratio << endl;
}





int read_high_scores(high_score *score) {

	ifstream infile;
	int squares, mines, time, i = 0;
	float ratio = 1e99;
	char name[MAX_INPUT_CHARACTERS];
	
	if (!infile_file(infile, "Scores")) 
		return i;
		
	infile >> name;
	
	if (strcmp(name, "Minesweeper_High_Scores") != 0)
		return ratio;
	
	for (i = 0; i < 10 && (infile >> score[i].name >> score[i].squares >> score[i].mines >> score[i].time >> score[i].ratio); ++i);		
	
	--i;
				
	infile.close();
	
	show_high_scores(score, i);
	
	return i;
}





void write_high_scores(char *winners_name, int mines, int time, high_score *score, int i) {

	ofstream outfile;
				
	outfile.open("Scores", ios::out | ios::trunc);
	
	if (outfile.fail()) {
		beep();
		cerr << "\nError: High Scores file could not be written!" << endl;
		return;
	}
	if (i < 9)
		++i;
	
	strcpy(score[i].name, winners_name);
	score[i].squares = num.h_boxes*num.v_boxes;
	score[i].mines = mines;
	score[i].time = time;
	score[i].ratio = time/(num.h_boxes*num.v_boxes/5);
	
	sort(score, i);
	
	outfile << "Minesweeper_High_Scores" << endl; 
		
	for (int j = 0; j < 10 && j <= i && outfile; ++j)
		outfile << score[j].name << " " << score[j].squares << " " << score[j].mines << " " << score[j].time << " " << score[j].ratio << endl; 
				
	if (!outfile)
		cout << "\nError: Out of space!" << endl;
	
	outfile.close();
	
	show_high_scores(score, i);
}





void  sort(high_score *score, int i)  {

	for (int j = 0; j <= i; ++j)
		for (int k = 0; k <= i - j - 1; ++k)
			if (score[k].ratio > score[k + 1].ratio)
				swap(score[k], score[k + 1]);	
}	





void swap(high_score &score1, high_score &score2) {

	high_score temp;

	temp = score1;
	score1 = score2;
	score2 = temp;
}










