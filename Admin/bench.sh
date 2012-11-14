#!/bin/sh
# meaningless index of performance statistics
# MIPS M120 => 9.2 user,
# so ``mips'' = ( 9.2 / u ) * 10
TMP=/tmp/dc.bench.$$
echo 'echo "99k2vdsap8op19^pla/pq" | /bin/time dc > /dev/null'
echo "99k2vdsap8op19^pla/pq" | /bin/time dc > /dev/null 2>$TMP
awk '
BEGIN			{
		secs = 0.0
	}
/[Rr]eal.*[Uu]ser.*[Ss]ys/	{
		secs = $3
	}
/^[Uu]ser/			{
		secs = $2
	}
END			{
		if ( secs > 0.0 )
			printf "user cpu secs: %.2f => ~mips %.1f\n", secs, 92 / secs
		else
			printf "could not recognise user seconds\n"
	}
' <$TMP
rm -f $TMP
