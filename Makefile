#Meedit 0.2 2020.05 (#3)

COMPILER = g++
ARGS = -lncurses
INPUT = Meedit/main.cpp
OUTPUT = meedit

all: Wellcome_message Build_meedit End_message

Wellcome_message:
	@echo "[Initializing...]"
	@echo " *** The following softwares will be build ***"
	@echo "==> Meedit 0.2 2020.05 (#3)"
	@echo "[Begginig build...]"

Build_meedit:
	@${COMPILER} ${INPUT} -o ${OUTPUT} ${ARGS} 

End_message:
	@echo "[Finalizing...]"
