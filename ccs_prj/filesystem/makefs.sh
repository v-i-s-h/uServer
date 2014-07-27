if [ -f filesystemImage.h ]
then
	rm filesystemImage.h
else
	echo "No filesystem image found!"
fi

./makefsfile -i $1 -o filesystemImage.h -r -h -q
