# Fetch the newest code
git init

git add --all

git remote add ramp git@github.com:vspup/oamp.git

git fetch ramp master 

git diff ramp/master --name-status

for file in `git diff ramp/master --name-status`
do
    rm -f -- "$file"
done

git diff ramp/master --name-status

git add --all

git pull ramp master