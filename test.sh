#!/bin/bash
# Insert your favourites terminal emulators here, in order (the first one found # in the system is used)
TERMINALS=( urxvt xterm )
# Do not change anything below this line if you don't know what you're doing!
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
		TERMINAL=`which ${TERMINALS[$i]}`
		let i=$i+1
		if [ "$i" == "${#TERMINALS[@]}" ]; then
			echo "No terminal emulators available."
			exit 1
		fi
	done

	$TERMINAL -e $@ &
}
fi
newin test/Monitor.exe
sleep 2
newin ./MobileLoadBalancer
sleep 1
newin ./FixedLoadBalancer
sleep 2
newin test/Appfixed.exe
sleep 1
newin test/Appmobile.exe
