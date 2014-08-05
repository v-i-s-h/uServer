if [ -f flashFs.h ]
then
	rm flashFs.h
else
	echo "No flash filesystem image found!"
fi

./makefsfile -i flashFs/ -o flashFs.h -r -h -q
