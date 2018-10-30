#### 提交说明
  大家提交的时候都push到自己的新分支，然后再向`dev` 分支提一个``` New pull request```
  
#### 代码规范
 1. 变量名采用全部小写，单词之间加下划线，过长的单词可以写成简写比如前三位或者去掉所有元音字母。比如``` player_curr_pos ```
 2. 函数名单词首字母大写。比如 ```GameStart()```
 3. 宏名全部字母大写，单词之间加下划线。比如 ```CHECK_OUT_PLAYER```
 4. 注意代码重用，较长的写成函数，较短的写成宏。
 5. 注意取名的意义，i，j 之类的变量随用随声明
 6. 所有变量声明在函数开头，``` i,j,k``` 之类除外
 7. 随手写注释，用英文。
 8. 标示写成宏，便于使用，比如 
  ```C
  #define HOSPITAL 'H'    // hospital
#define TOOL 'T'        // tools
#define SPACE '0'       // space
#define GIFT 'G'        // gift house
#define MAGIC 'M'       // magic house
#define PRISON 'P'      // prison
#define MINERAL '$'     // mineral area
#define START 's'       // start area
  ```
 9. 尽量减少代码嵌套的层数。

#### 编译
``` cmake .```

``` make ```

#### 运行
``` ./richman ```
