#include "head.h"
#include "md5.h"
#define SALT_LEN 10

void makesalt(char* str)
{
    int i,flag;
    srand(time(NULL));
    for(i=0;i<SALT_LEN;i++){
        flag=rand()%3;
        switch(flag){
        case 0:
            str[i]=rand()%26+'a';
            break;
        case 1:
            str[i]=rand()%26+'A';
            break;
        case 2:
            str[i]=rand()%10+'0';
            break;
        }
    }
}

int sign_in(int newfd)
{
    Train_t userName,Salt,Ciphe;
    int ret,dataLen;
    char *passwd,*ciphetext;
    char buf[1000]={0};
    char name[20]={0};
    char str[SALT_LEN+1]={0};
    char salt[100]="$6$";
    printf("请输入新用户名(20个字以内)：");
    scanf("%s",name);
    //发送用户名小火车
    userName.dataLen=strlen(name);
    strcpy(userName.buf,name);
    ret=sendCycle(newfd,(char*)&userName,4+userName.dataLen);
    ERROR_CHECK(ret,-1,"senduserName");
    
    //接收是否重名信息
    recvCycle(newfd,(char*)&dataLen,4);
    recvCycle(newfd,buf,dataLen);
    printf("sameFlag = %d\n",dataLen);
    if(!dataLen){
        printf("该用户名已存在,请登录或选择其他用户名注册\n");
        sleep(5);
        return 1;
    }
    else{
        printf("该用户名可以使用\n");
    }

    makesalt(str);//生成随机数
    strcat(salt,str);//拼接成盐值
    printf("salt = %s , len = %ld\n",salt,strlen(salt));
    passwd=getpass("请输入新密码：");
    if(NULL==passwd){
        printf("密码有误,请重新选择");
        sleep(5);
        return -1;
    }
    printf("passwd = %s , len = %ld\n",passwd,strlen(passwd));
    ciphetext=crypt(passwd,salt);//生成密文
    printf("cipetext = %s , len = %ld\n",ciphetext,strlen(ciphetext));
    
    //发送盐值小火车
    Salt.dataLen=strlen(salt);
    strcpy(Salt.buf,salt);
    printf("In the Salt strut %d %s\n",Salt.dataLen,Salt.buf);
    ret=sendCycle(newfd,(char*)&Salt,4+Salt.dataLen);
    ERROR_CHECK(ret,-1,"sendsalt");
    //发送密文小火车
    Ciphe.dataLen=strlen(ciphetext);
    strcpy(Ciphe.buf,ciphetext);
    printf("In the Ciphe struct %d %s\n",Ciphe.dataLen,Ciphe.buf);
    ret=sendCycle(newfd,(char*)&Ciphe,4+Ciphe.dataLen);
    ERROR_CHECK(ret,-1,"sendciphe");
    
    printf("注册成功，5秒后请重新选择注册或登录账号");
    sleep(5);
    return 0;
}

int log_in(int newfd,char* NAME)
{
    Train_t userName,Ciphe;
    int ret,dataLen;
    char *passwd,*ciphetext;
    char buf[1000]={0};
    char salt[512]={0};
    char name[20]={0};
    bzero(&userName,sizeof(userName));
    bzero(&Ciphe,sizeof(Ciphe));
    printf("请输入用户名(20个字以内)：");
    scanf("%s",name);
    //发送用户名小火车
    userName.dataLen=strlen(name);
    strcpy(userName.buf,name);
    ret=sendCycle(newfd,(char*)&userName,4+userName.dataLen);
    ERROR_CHECK(ret,-1,"senduserName");
    
    //接收是否存在该用户名信号
    recvCycle(newfd,(char*)&dataLen,4);
    recvCycle(newfd,buf,dataLen);
    printf("sameFlag = %d\n",dataLen);
    if(dataLen){}
    else{
        printf("该用户名不存在,5秒后请重新选择登录或注册\n"); 
        return 1;
    }

    recvCycle(newfd,(char*)&dataLen,4);//接收盐值大小
    recvCycle(newfd,salt,dataLen);//接收盐值
    printf("salt = %s , len = %ld\n",salt,strlen(salt));
    
    passwd=getpass("请输入密码：");
    if(NULL==passwd){
        printf("密码有误");
        return -1;
    }
    printf("passwd = %s , len = %ld\n",passwd,strlen(passwd));
    
    ciphetext=crypt(passwd,salt);//跟传来的盐值生成密文
    printf("cipetext = %s , len = %ld\n",ciphetext,strlen(ciphetext));
    //发送密文小火车
    Ciphe.dataLen=strlen(ciphetext);
    strcpy(Ciphe.buf,ciphetext);
    printf("In the Ciphe struct %d %s\n",Ciphe.dataLen,Ciphe.buf);
    ret=sendCycle(newfd,(char*)&Ciphe,4+Ciphe.dataLen);
    ERROR_CHECK(ret,-1,"sendciphe");
    
    //接收是否登录成功信号
    bzero(buf,sizeof(buf));
    recvCycle(newfd,(char*)&dataLen,4);
    recvCycle(newfd,buf,dataLen);
    printf("passflag = %d\n",dataLen);
    if(0==dataLen){
        printf("密码错误，5秒后请重新选择注册或登录\n");
        sleep(5);
        return 1;
    }
    strcpy(NAME,name);//传入传出
    printf("登录成功，欢迎您 %s\n",name);
    sleep(2);
    return 0;
}

int user_ctr(int newfd,char* name)
{
    Train_t accessFlag,Command,md5;
    int ret,dataLen,i,j,k,n,N;
    int slashNum=2;
    char buf[1000]={0};
    char path[512]={0};
    char command[64]={0};
    char mode[32]={0};
    char filename[32]={0};
    char md5_str[MD5_STR_LEN + 1];
    char *spaceFlag;
    system("clear");
    sprintf(path,"/%s/",name);
    while(1){
        bzero(&accessFlag,sizeof(accessFlag));
        bzero(&buf,sizeof(buf));
        bzero(&Command,sizeof(Command));
        bzero(&md5,sizeof(md5));
        bzero(md5_str,sizeof(md5_str));
        printf("\n%s\n -> ",path);
        __fpurge(stdin);//清除输入缓存
        fgets(command,sizeof(command),stdin);
        //删除末尾的换行符
        if(command[strlen(command)-1]=='\n'){
            command[strlen(command)-1]='\0';
        }
        Command.dataLen=strlen(command);
        strcpy(Command.buf,command);
        ret=sendCycle(newfd,(char*)&Command,4+Command.dataLen);
        ERROR_CHECK(ret,-1,"sendcommand");
       
        spaceFlag=strchr(command,32);
        if(NULL==spaceFlag){//该命令不带空格
            //打印当前目录下文件列表
            if(!strcmp(command,"ls")){
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                printf("%s",buf);
            }
            else printf("您输入的命令不合法\n");
        }
        
        else{//该命令带空格
            for(i=0;command[i]!=' ';i++){
                mode[i]=command[i];
            }
            mode[i]='\0';
            for(i++,j=0;command[i]!='\0';i++,j++){
                filename[j]=command[i];
            }
            filename[j]='\0';

            //创建目录功能
            if(!strcmp(mode,"mkdir")){
                //接收目录是否重名信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(dataLen){
                    printf("目录 %s 创建成功\n",filename);
                }
                else{
                    printf("创建失败,该目录已存在,请重新输入\n");
                }
            }
            
            //删除目录功能
            else if(!strcmp(mode,"rmdir")){
                //接收目录是否存在信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(dataLen){
                    printf("目录 %s 删除成功\n",filename);
                }
                else{
                    printf("删除失败,该目录不存在,请重新输入\n");
                }
            }

            //进入目录功能
            else if(!strcmp(mode,"cd")){
                //接收进入上级或下级和目录名是否存在信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(1==dataLen){
                    printf("进入目录 %s 成功\n",filename);
                    sprintf(path,"%s%s/",path,filename);
                    slashNum++;
                }
                else if(2==dataLen){
                    printf("进入上级目录成功\n");
                    for(N=slashNum,k=0,n=0;n!=N-1;k++){
                       if(path[k]=='/')n++; 
                    }
                    for(;k!=512;k++){
                        path[k]='\0';
                    }
                    slashNum--;
                }
                else if(3==dataLen){
                    printf("该目录已是根目录,无法向上,请重输\n");
                }
                else{
                    printf("进入失败,该目录不存在,请重新输入\n");
                }
            }
            
            //上传文件功能
            else if(!strcmp(mode,"puts")){
                accessFlag.dataLen=1;
                //判断本机端是否存在该文件
                if(-1==access(filename,F_OK)){//若不存在该文件
                    printf("本机端不存在该文件，无法上传\n");
                    accessFlag.dataLen=0;
                    ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                    ERROR_CHECK(ret,-1,"accessflag0");
                    continue;
                }
                ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                ERROR_CHECK(ret,-1,"accessflag1");
                //接收同名文件是否存在信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(1==dataLen){
                    //发送md5码
                    ret = Compute_file_md5(filename, md5_str);
                    if (0 == ret)
                    {
                    	printf("[file - %s] md5 value: ",filename);
                    	printf("%s\n", md5_str);
                    }
                    md5.dataLen=strlen(md5_str);
                    strcpy(md5.buf,md5_str);
                    ret=sendCycle(newfd,(char*)&md5,4+md5.dataLen);
                    ERROR_CHECK(ret,-1,"sendmd5");
                    //接收是否传送文件内容信号
                    recvCycle(newfd,(char*)&dataLen,4);
                    recvCycle(newfd,buf,dataLen);
                    if(1==dataLen){
                        printf("开始上传文件内容\n");
                        ret=tranFile(newfd,filename);
                        ERROR_CHECK(ret,-1,"getfile");
                        if(0==ret)printf("文件 %s 上传成功\n",filename);
                    }
                    else{
                        printf("服务器上已有文件内容,不再传输\n");
                    }
                }
                else{
                    printf("已有同名文件存在,请重新输入\n");
                }
            }
            
            //下载文件功能
            else if(!strcmp(mode,"gets")){
                accessFlag.dataLen=1;
                //判断本机端是否存在该文件
                if(-1==access(filename,F_OK)){//若不存在该文件，则正常下载
                    ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                    ERROR_CHECK(ret,-1,"accessflag1");
                }
                else{//若已存在该文件，则不下载或断点下载
                    accessFlag.dataLen=0;
                    ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                    ERROR_CHECK(ret,-1,"accessflag0");
                    printf("本机端已存在该文件\n");
                    accessFlag.dataLen=0;
                    ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                    ERROR_CHECK(ret,-1,"accessflag0");
                    continue;
                }
                ret=sendCycle(newfd,(char*)&accessFlag,4+accessFlag.dataLen);
                ERROR_CHECK(ret,-1,"accessflag1");
                //接收该文件是否存在网盘当前目录信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(1==dataLen){//开始下载
                        ret=getFile(newfd);
                        ERROR_CHECK(ret,-1,"getfile");
                        if(0==ret)printf("文件 %s 下载成功\n",filename);
                }
                else{
                    printf("当前目录下不存在该文件,请重新输入\n");
                }
            }
        }
    } 
}
