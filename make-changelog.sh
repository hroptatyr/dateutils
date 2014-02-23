#!/bin/zsh

cat <<'EOF'
---
title: dateutils changelog
layout: subpage
project: dateutils
logo: dateutils_logo_120.png
---

EOF

for i in `git tag | grep -vF 'rc' | tac`; git tag -l -n1000 ${i} | sed 's/v\([0-9]\.[.0-9]*\)  */\nv\1\n=======\n    /'
