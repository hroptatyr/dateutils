#!/bin/zsh

cat <<'EOF'
---
title: dateutils changelog
layout: subpage
project: dateutils
logo: dateutils_logo_120.png
---

EOF

for i in `git tag | grep -vF 'rc
beta' | tac`; \
    git tag -l -n1000 ${i} | "$(dirname "${0}")/make-changelog.sed"
