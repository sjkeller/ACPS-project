# Quick Overview (Git)

###  Learn about the concepts of Git

- [5 Minute Overview of Git](https://www.youtube.com/watch?v=xvwBtODV0ms)
- [Git Explained in 100 Seconds](https://www.youtube.com/watch?v=hwP7WQkmECE)
- [Pro Git Book](https://git-scm.com/book/en/v2)

### GitLab

- [https://collaborating.tuhh.de/](https://collaborating.tuhh.de/) 
  - This is the URL to access the GitLab instance hosted by TUHH Rechenzentrum.
- [Create first sample-project](https://docs.gitlab.com/ee/tutorials/make_your_first_git_commit.html#create-a-sample-project)
- [Dark Mode?!](https://collaborating.tuhh.de/-/profile/preferences)


## Install Git

There are different methods how Git can be used:
- Native [command line tool](https://git-scm.com/) (Git Bash)
- Desktop Client with GUI, e.g., [GitHub Desktop Client](https://desktop.github.com/) 
- Integrated Git Client in IDE (VS Code, Mbed Studio)

Typically, you have to decide on your own, which method works for you best. 

My personal recommendation is to learn the basic concepts of Git using the command line tool.  You will benefit from it later when looking for solutions to problems. Also, certain configurations are often easier to do with the Git command line tool (see the example for setting up the SSH key).

Using a graphical user interfaces can be more productive and very useful for advanced tasks, e.g., when analyzing the diff, resolving conflicts in the codebase, perform merges  or cherry picking. Therefore, I also recommend using the GitHub Desktop Client in combination/addition to the native command line tool. Again, it is up to you which Git method works for you best.

### Install Git command line tool

- [Download and install Git command line tool](https://git-scm.com/)
  - There are a lot of options during the installation but you can simply leave the default selection for all questions, except when asked for the editor we recommend to use notepad++.

### Install GitHub desktop client

Can be used for any repository regardless whether it is hosted on GitHub or GitLab.

- [Download and install GitHub Desktop Client](https://desktop.github.com/) (Windows/Mac)
  - Currently, Github Desktop for **GNU/Linux** is not officially supported by the GitHub, but there is a fork that hosts a version for some Linux Distributions (see [here](https://dev.to/rahedmir/is-github-desktop-available-for-gnu-linux-4a69) and [here](https://github.com/shiftkey/desktop/releases))
- If you have already cloned a repository using Git Bash then you can simply drag and drop the repository folder into the GitHub Desktop GUI.



## Basic commands

- see also [git-cheat-sheet.pdf](https://about.gitlab.com/images/press/git-cheat-sheet.pdf)

```shell
# show help for all commands
git help

git clone

git add <file>

# (all files in current directory and subdirectories)
git add . 

git commit -m "text"

git push

git pull

git status

git reset
```

