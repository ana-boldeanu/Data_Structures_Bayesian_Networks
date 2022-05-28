#!/bin/bash
#//////////////////////////////////////////////////////
#	Created by Nan Mihai on 14.05.2020
#	Copyright (c) 2021 Nan Mihai. All rights reserved.
#	Checker Tema 3 - Structuri de date
#	Rețele Bayesiene
#	Facultatea de Automatica si Calculatoare
#	Anul Universitar 2020-2021, Seria CD
#/////////////////////////////////////////////////////
print_header()
{
	header="${1}"
	header_len=${#header}
	printf "\n"
	if [ $header_len -lt 71 ]; then
		padding=$(((71 - $header_len) / 2))
		for ((i = 0; i < $padding; i++)); do
			printf " "
		done
	fi
	printf "= %s =\n\n" "${header}"
}

result1=0
result2=0
result=0
depunctari=0
ZERO=0
VALGRIND="valgrind --leak-check=full --track-origins=yes -q --log-file=rezultat_valgrind.txt"
BEST="100"

print_header "Testare - Tema 3 (SD-CD)"

if test -f "README"; then
	echo "+0.0: Verificare README ................................................. PASS"
	result=$(awk "BEGIN {print $result+10; exit}")
else
	echo "-10.0: Verificare README ................................................ FAIL"
fi
make build > /dev/null 2>&1

#Cerința 1 - Verificarea Rețelei Bayesiene
echo "Cerința 1 - Verificarea Rețelei Bayesiene"
for i in {0..9}
do
	fileIn="teste/cerinta1/test"$i".in"
	fileRef="teste/cerinta1/test"$i".ref"
	fileOut="bnet.out"
	cp "$fileIn" "bnet.in"
	timeout 20 ./bnet -c1 > /dev/null 2>&1
	diff $fileOut $fileRef > /dev/null
	EXIT_CODE=$?
	if (( i < 10 )); then
		idx=" $i"
	else
		idx=$i
	fi
	if [ $EXIT_CODE -eq $ZERO ] 
	then
		echo "+0.0: Cerinta 1 Test $idx ................................................. PASS"
		result=$(awk "BEGIN {print $result+3; exit}")
		result1=$(awk "BEGIN {print $result1+3; exit}")
	else
		echo "-1.0: Cerinta 1 Test $idx ................................................. FAIL"
	fi
done

#Cerința 2 - D-Separabilitate
echo "Cerința 2 - D-Separabilitate"
for i in {0..9}
do
	fileIn="teste/cerinta2/test"$i".in"
	fileOut="bnet.out"
	fileRef="teste/cerinta2/test"$i".ref"
	cp "$fileIn" "bnet.in"
	timeout 20 ./bnet -c2 > /dev/null 2>&1
	diff $fileOut $fileRef > /dev/null
	EXIT_CODE=$?
	if (( i < 10 )); then
		idx=" $i"
	else
		idx=$i
	fi
	if [ $EXIT_CODE -eq $ZERO ] 
	then
		echo "+0.0: Cerinta 2 Test $idx ................................................. PASS"
		result=$(awk "BEGIN {print $result+6; exit}")
		result2=$(awk "BEGIN {print $result2+6; exit}")
	else
		echo "-1.0: Cerinta 2 Test $idx ................................................. FAIL"
	fi
done

echo "Verificarea dealocării memoriei"
fileIn="teste/cerinta1/test9.in"
fileOut="bnet.out"
cp "$fileIn" "bnet.in"
$VALGRIND ./bnet -c1 > /dev/null 2>&1
if [[ -z $(cat rezultat_valgrind.txt) ]]; then
	printf "+0.0: VALGRIND Cerința 1 ................................................ PASS\n"
else
	depunctare=$(echo "scale=1; -($result1/10)" | bc -l)
	printf -- "$depunctare: VALGRIND Cerinta 1 ................................................ FAIL\n"
	result=$(awk "BEGIN {print $result-($result1/10); exit}")
	depunctari=$(awk "BEGIN {print $depunctari+($result1/10); exit}")
fi
fileIn="teste/cerinta2/test9.in"
fileOut="bnet.out"
cp "$fileIn" "bnet.in"
$VALGRIND ./bnet -c2 > /dev/null 2>&1
if [[ -z $(cat rezultat_valgrind.txt) ]]; then
	printf "+0.0: VALGRIND Cerința 2 ................................................ PASS\n"
else
	depunctare=$(echo "scale=1; -($result2/10)" | bc -l)
	printf -- "$depunctare: VALGRIND Cerinta 2 ................................................ FAIL\n"
	result=$(awk "BEGIN {print $result-($result2/10); exit}")
	depunctari=$(awk "BEGIN {print $depunctari+($result2/10); exit}")
fi

echo "Rezultate"
printf "Cerința 1 : "$result1"\n"
printf "Cerința 2 : "$result2"\n"
printf "Depunctări: "$depunctari"\n"
if (( result == BEST )); then
	echo "Felicitări! Ai punctajul maxim: "$BEST"p! :)"
else
	echo "Ai acumulat "$result"p din maxim 100p! :("
fi
make clean > /dev/null 2>&1
