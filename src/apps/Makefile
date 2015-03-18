include ../common.mk

# flags for building the gtk library
CFLAGS = $(CFLAGS_STD) \
	 $(CFLAGS_VX_GTK) \
     $(CFLAGS_IMAGESOURCE) \
	 $(CFLAGS_MATH) \
	 $(CFLAGS_COMMON) \
	 $(CFLAGS_LCMTYPES) \
	 -O2

CXXFLAGS = $(CXXFLAGS_STD) \
	 $(CFLAGS_VX_GTK) \
     $(CFLAGS_IMAGESOURCE) \
	 $(CFLAGS_MATH) \
	 $(CFLAGS_COMMON) \
	 $(CFLAGS_LCMTYPES) \
	 -O2

LDFLAGS = $(LDFLAGS_STD) \
	  $(LDFLAGS_VX_GTK) \
	  $(LDFLAGS_IMAGESOURCE) \
	  $(LDFLAGS_MATH) \
	  $(LDFLAGS_COMMON) \
	  $(LDFLAGS_LCMTYPES) \
	  $(LDFLAGS_GSL)


LIBDEPS = $(call libdeps, $(LDFLAGS))

LIB_EECS467 = $(LIB_PATH)/libeecs467.a
LIBEECS467_OBJS = eecs467_util.o

BIN_EECS467_GUI_EXAMPLE = $(BIN_PATH)/eecs467_gui_example
BIN_EECS467_REXARM_EXAMPLE = $(BIN_PATH)/eecs467_rexarm_example
BIN_EECS467_MAEBOT_EXAMPLE = $(BIN_PATH)/eecs467_maebot_example
BIN_EECS467_MAEBOT_VIDEO_TELOP = $(BIN_PATH)/eecs467_maebot_video_telop

BIN_EECS467_A2_COLOR_PICKER = $(BIN_PATH)/a2_color_picker
BIN_EECS467_A2_AI = $(BIN_PATH)/a2_ai
BIN_EECS467_A2_PICK_PLACE = $(BIN_PATH)/a2_pick_place
BIN_EECS467_A2_AI = $(BIN_PATH)/a2_ai
BIN_EECS467_A2_INVERSE_KINEMATICS = $(BIN_PATH)/a2_inverse_kinematics
BIN_EECS467_A2_BOARD_STATE = $(BIN_PATH)/a2_board_state
BIN_EECS467_A2_MASK = $(BIN_PATH)/a2_mask
BIN_EECS467_A2_TTT = $(BIN_PATH)/a2_ttt
BIN_EECS467_A2_CALI = $(BIN_PATH)/a2_cali

ALL = $(LIB_EECS467) $(BIN_EECS467_GUI_EXAMPLE) $(BIN_EECS467_REXARM_EXAMPLE) \
      $(BIN_EECS467_MAEBOT_EXAMPLE) $(BIN_EECS467_MAEBOT_VIDEO_TELOP) \
      $(BIN_EECS467_A2_COLOR_PICKER) $(BIN_EECS467_A2_AI) $(BIN_EECS467_A2_PICK_PLACE) \
      $(BIN_EECS467_A2_INVERSE_KINEMATICS) $(BIN_EECS467_A2_BOARD_STATE) $(BIN_EECS467_A2_MASK) \
      $(BIN_EECS467_A2_IMAGE_TO_ARM) $(BIN_EECS467_A2_TTT) $(BIN_EECS467_A2_CALI)

all: $(ALL)

$(LIB_EECS467): $(LIBEECS467_OBJS) $(LIBDEPS)
	@echo "\t$@"
	@ar rc $@ $(LIBEECS467_OBJS)

$(BIN_EECS467_GUI_EXAMPLE): gui_example.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CC) -o $@ $^ $(LDFLAGS) $(LIB_EECS467)

$(BIN_EECS467_REXARM_EXAMPLE): rexarm_example.o $(LIBDEPS)
	@echo "\t$@"
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_EECS467_MAEBOT_EXAMPLE): maebot_example.o $(LIBDEPS)
	@echo "\t$@"
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_EECS467_MAEBOT_VIDEO_TELOP): maebot_video_teleop.o $(LIBDEPS)
	@echo "\t$@"
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BIN_EECS467_A2_COLOR_PICKER): a2_color_picker.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_AI): test.o a2_ai.o board.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_INVERSE_KINEMATICS): test3.o a2_inverse_kinematics.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_PICK_PLACE): test2.o a2_pick_place.o a2_inverse_kinematics.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_BOARD_STATE): test4.o a2_board_state.o a2_blob_detector.o board.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_MASK): a2_mask.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_IMAGE_TO_ARM): a2_image_to_arm.o a2_blob_detector.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_TTT): a2_ttt.o a2_board_state.o a2_blob_detector.o board.o a2_pick_place.o a2_inverse_kinematics.o a2_image_to_arm_coord.o a2_ai.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)

$(BIN_EECS467_A2_CALI): a2_cali.o a2_board_state.o a2_blob_detector.o board.o a2_pick_place.o a2_inverse_kinematics.o a2_image_to_arm_coord.o a2_ai.o $(LIB_EECS467) $(LIBDEPS)
	@echo "\t$@"
	@$(CXX) -o $@ $^ $(LDFLAGS) $(CXXFLAGS)


clean:
	@rm -f *.o *~ *.a
	@rm -f $(ALL)
