#! /bin/sh

ctags -e -a `find . -type f -name \*.[hcS]`
for d in `find . -type d`
do
    ctags -R -e "$d" > "$d/TAGS"
done

#! /bin/sh

find . -type d -exec ./dirtags.sh {} \;
ctags --file-scope=no -R

#find . -type f -iname "*.[chS]" | xargs etags -a