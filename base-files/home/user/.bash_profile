# If .bash_profile exists, bash doesn't read .profile
if [[ -f ~/.profile ]]; then
  . ~/.profile
fi

# If .bashrc exists, get the aliases and functions
[[ -f ~/.bashrc ]] && . ~/.bashrc
