# Practice_project
    用来学习一些高质量的项目。注意：git在提交文件的时候如果文件夹是中文的，那么会被重新编码。
## 命令git(可以在仓库的下一级)：
1. 本地项目发布到github中。
    - > 1) 将本地文件家初始化为仓库：git init
    - > 2) 添加文件到本地仓库：git add .
    - > 3) 提交文件到本地仓库：git commit -m “备注信息”
    - > 4) 添加远程仓库到本地仓库(只要初始化的时候做就行)：git remote add origin {远程仓库地址}
    - > 5) 获取远程仓库与本地同步合并（只要初始化的时候做就行）：git pull --rebase origin master
    - > 6) push 到远程仓库：git push -u origin master
2. 同步github仓库内容到本地：
    - > 命令一：git pull 将提交到默认分支。

## 文件.gitignore
    只要写上要忽略的文件就行，或者是通配符类型的。如：*.o。

## cJSON学习项目
