#Meedit 0.2 2020.05 (#3)

COMPILER = g++
ARGS = -lncurses
INPUT = Meedit_SRC/main.cpp
OUTPUT = Meedit
CFOLDER1 = technolandia_meedit/
CFOLDER2 = ~/.technolandia_meedit

all: Wellcome_message Setup_folder Build_meedit End_message

Wellcome_message:
	@echo "[Initializing...]"
	@echo "*** The following softwares will be build ***"
	@echo "==> Meedit 0.2 2020.05 (#3)"
	@echo "*** The following directories will be created***"
	@echo "==> "${CFOLDER2}

Setup_folder:
	@echo "[Setting up files...]"
	@mkdir ${CFOLDER2}
	@cp ${CFOLDER1}/* ${CFOLDER2}/ 

Build_meedit:
	@echo "[Begginig build...]"
	@${COMPILER} ${INPUT} -o ${OUTPUT} ${ARGS} 

End_message:
	@echo "[Finalizing...]"


