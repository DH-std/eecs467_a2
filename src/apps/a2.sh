#!/bin/bash

bin_path='../../bin'

#$(rm mask_board)
#$(rm mask_pick)

echo -e "\e[1;31mdefine the mask for the board \e[0m"
./$bin_path/a2_mask mask_board
echo -e "\e[1;31mdefine the mask for free ball pool \e[0m"
./$bin_path/a2_mask mask_pick

echo -e "\e[1;31mdefine red range \e[0m"
./$bin_path/a2_color_picker -o hsv_R
echo -e "\e[1;31mdefine green range \e[0m"
./$bin_path/a2_color_picker -o hsv_G
echo -e "\e[1;31mdefine cyan range \e[0m"
./$bin_path/a2_color_picker -o hsv_B

# cali arm

./$bin_path/a2_board_state

a="d"
while [ "$a" != "q" ]
do
    echo "more fun~" # play game
    echo -e "\e[1;31mtype q to quit \e[0m"
    read a
done

echo -e "\e[1;31mSee \e[0;33myou \e[1;34msoon \e[1;35magain~ \e[0m"