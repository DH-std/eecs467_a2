
#include <string>
#include <iostream>
#include "a2_ai.h"
#include "except.h"





int main() {

    string s =  "R.G"
                "G.R"
                ".R.";
    int x = 3;
    int y = 3;

    char player = 'R';

    a2_ai ai(s, player);

    ai.print_state();

    int i = 0;
    while (i < 10 && x != -1 && y != -1) {
        cout << endl << "################loop# " << i << " turn " << player << endl;

        try {

            ai.next_move(x, y);
        }
        catch (MyException s) {
            cout << s.what() << endl;
            exit(-1);
        }
        cout << "################returned move " << x << "," << y << " turn " << player << endl;

        ai.board.update_cell(x, y, player);

        ai.print_state();

        player = ai.opposite(player);
        ai.set_turn(player);

        ++i;
    }

}