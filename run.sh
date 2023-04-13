#!/bin/bash

# get url of current origin
OLD_ORIGIN=$(git config --get remote.origin.url)

# set remote url to template repo
git remote set-url origin git@github.com:OSU-OS2-S23/JOS-Labs.git

# fetch all branches (include commit history)
git fetch

# set remote back to old origin
git remote set-url origin $OLD_ORIGIN

# individually checkout to each branch and push
git checkout master
git push


for branch in lab1 lab2 lab3 lab4
do
  git checkout $branch
  git push
done
