## Using Git with SSH Keys

*SSH keys* are used to authenticate secure connections. By linking an SSH Key with your git account (GitLab or GitHub) you are able to clone, pull and push from remote repositories without password authentication. 

This can be useful if you set up repositories on shared/external machines where you do not want your passwords to be seen by others. Also, you know that the *TUHH Rechenzentrum* requires you to change your password every 6 months, which also affects your GitLab Account. So by using SSH, you avoid having to update your password regularly in your Git client as well.

### How to create an SSH key and link to your git account

- Open the git terminal and enter:

    ~~~bash
    ssh-keygen -o -t rsa -C "your@email.com"
    ~~~

- Confirm all questions by just **pressing enter**

- Copy the output string, which follows after entering:

    ~~~bash
    cat ~/.ssh/id_rsa.pub	
    ~~~

- Finally, you can link the key to your

    - [GitLab Account](https://collaborating.tuhh.de/-/profile/keys) (Profile/Preferences/SSH Keys)
    - [GitHub Account](https://github.com/settings/keys)

