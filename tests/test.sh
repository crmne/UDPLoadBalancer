#!/bin/bash
# ------------------------
# Insert your favourites terminal emulators here, in order (the first one found in the system is used)
TERMINALS=( urxvt xterm )
# Programs to run, in order
PROGRAMS="disttest/Monitor.exe ./mlb ./flb disttest/Appfixed.exe disttest/Appmobile.exe"
# Delay in seconds between the execution of each program
DELAY=1
# ------------------------
# Do not change anything below this line if you don't know what you're doing!
# ------------------------
if [ "`uname`" == "Darwin" ]; then
function newin() {
   CWD=`/bin/pwd`
   /usr/bin/open -a Terminal
   /usr/bin/osascript <<__END__
   tell application "Terminal" to do script with command "cd $CWD; $@"
__END__
}
else
function newin() {
	TERMINAL=""
	i=0
	while [ "$TERMINAL" == "" ]; do
		if [ "$i" == "${#TERMINALS[@]}" ]; then
			echo "No terminal emulators available."
			exit 1
		fi
		TERMINAL=`which ${TERMINALS[$i]} 2> /dev/null`
		let i=$i+1
	done

	$TERMINAL -e $@ &
}
fi

for prog in $PROGRAMS; do
		echo Starting $prog ...
		newin $prog
		sleep $DELAY
done
