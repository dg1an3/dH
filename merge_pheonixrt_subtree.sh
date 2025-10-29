#!/bin/bash

# Update the merge_pheonixrt_subtree.sh script to add the phoenixrt subtree without --squash

# Command to add the phoenixrt subtree with full history

git subtree add --prefix=phoenixrt https://github.com/your-repo/phoenixrt.git main
