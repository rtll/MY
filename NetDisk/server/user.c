#include "factory.h"
#include<mysql/mysql.h>
int sign_in(int newfd)
{
    printf("将有用户注册\n");
    int dataLen,ret;
    char name[512]={0};
    char salt[512]={0};
    char ciphe[512]={0};
    Train_t sameFlag;
    bzero(&sameFlag,sizeof(sameFlag));

    //调用数据库
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char server[20]="localhost";
    char user[20]="root";
    char password[20]="290010";
    char database[20]="net_disk";
    char query[1024]={0};
    conn=mysql_init(NULL);
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){//连接数据库
        printf("Error connecting to database:%s\n",mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    else{
        printf("Connet database success\n");
    }

    recvCycle(newfd,(char*)&dataLen,4);//接收用户名大小
    recvCycle(newfd,name,dataLen);//接收用户名
    printf("name : %s , len : %d\n",name,dataLen);
    sprintf(query,"select * from user where Name='%s'",name);
    ret=mysql_query(conn,query);
    if(ret){
        printf("Error making query:%s\n",mysql_error(conn));
    }
    else{
        res=mysql_use_result(conn);
        if(res){
            if((row=mysql_fetch_row(res))){//如果找到了相同用户名的条目
                printf("数据库中已有相同用户名\n");
                mysql_free_result(res);
                sameFlag.dataLen=0;
                ret=sendCycle(newfd,(char*)&sameFlag,4+sameFlag.dataLen);//发送重名信息给客户端
                ERROR_CHECK(ret,-1,"sendsameflag0");
                return -1;
            }
            else{
                sameFlag.dataLen=1;
                ret=sendCycle(newfd,(char*)&sameFlag,4+sameFlag.dataLen);//发送用户名可用信息
                ERROR_CHECK(ret,-1,"sendsameflag1");
            }
        }
        mysql_free_result(res);
    }

    recvCycle(newfd,(char*)&dataLen,4);//接收盐值大小
    recvCycle(newfd,salt,dataLen);//接收盐值
    printf("salt : %s , len : %d\n",salt,dataLen);
    recvCycle(newfd,(char*)&dataLen,4);//接收密文大小
    recvCycle(newfd,ciphe,dataLen);//接收密文
    printf("ciphe : %s , len : %d\n",ciphe,dataLen);

    
    //数据库中user表新建行-用户账号信息
    bzero(query,sizeof(query));
    sprintf(query,"insert into user (Name,Salt,Ciphetext) values ('%s','%s','%s')",name,salt,ciphe);
    ret=mysql_query(conn,query);//传命令，成功返回0，失败非零
    if(ret){
        printf("Error making query:%s\n",mysql_error(conn));
        return -1;
    }
    else{
        printf("用户新增成功\n");
    }

    //数据库中file表新建行-用户主目录
    bzero(query,sizeof(query));
    sprintf(query,"insert into file (procode,filename,belong,filetype) values (0,'%s','%s','dir')",name,name);
    ret=mysql_query(conn,query);
    if(ret){
        printf("Error making query:%s\n",mysql_error(conn));
        return -1;
    }
    else{
        printf("用户主目录创建成功\n");
    }
    mysql_close(conn);
    return 0;
}

int log_in(int newfd,char *NAME)
{
    printf("将有用户登录\n");
    int dataLen,ret;
    char name[20]={0};
    char salt[512]={0};
    char CIPHE[512]={0};
    char ciphe[512]={0};
    Train_t sameFlag,Salt,passFlag;
    bzero(&sameFlag,sizeof(sameFlag));
    bzero(&Salt,sizeof(Salt));
    bzero(&passFlag,sizeof(passFlag));

    //调用数据库
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char server[20]="localhost";
    char user[20]="root";
    char password[20]="290010";
    char database[20]="net_disk";
    char query[1024]={0};
    conn=mysql_init(NULL);
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){//连接数据库
        printf("Error connecting to database:%s\n",mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    else{
        printf("Connet database success\n");
    }
    
    //接收用户名
    recvCycle(newfd,(char*)&dataLen,4);//接收用户名大小
    recvCycle(newfd,name,dataLen);//接收用户名
    printf("name : %s , len : %d\n",name,dataLen);
    sprintf(query,"select * from user where Name='%s'",name);
    ret=mysql_query(conn,query);
    if(ret){
        printf("Error making query:%s\n",mysql_error(conn));
    }
    else{
        res=mysql_use_result(conn);
        if(res){
            if((row=mysql_fetch_row(res))){//如果找到了相同用户名的条目
                sameFlag.dataLen=1;
                //获得该用户名对应的盐值和密文
                strcpy(salt,row[2]);
                strcpy(ciphe,row[3]);
                printf("salt : %s , len : %ld\n",salt,strlen(salt));
                printf("ciphe : %s , len : %ld\n",ciphe,strlen(ciphe));
                ret=sendCycle(newfd,(char*)&sameFlag,4+sameFlag.dataLen);//发送用户名正确信息给客户端
                ERROR_CHECK(ret,-1,"sendsameflag0");
            }
            else{
                printf("数据库中不存在该用户名\n");
                sameFlag.dataLen=0;
                ret=sendCycle(newfd,(char*)&sameFlag,4+sameFlag.dataLen);//发送用户名不存在信息
                ERROR_CHECK(ret,-1,"sendsameflag1");
                mysql_free_result(res);
                return -1;
            }
        }
        mysql_free_result(res);
    }

    Salt.dataLen=strlen(salt);
    strcpy(Salt.buf,salt);
    ret=sendCycle(newfd,(char*)&Salt,4+Salt.dataLen);//发送盐值
    ERROR_CHECK(ret,-1,"sendsalt");

    recvCycle(newfd,(char*)&dataLen,4);//接收密文大小
    recvCycle(newfd,CIPHE,dataLen);//接收密文
    printf("CIPHE : %s , len : %d\n",CIPHE,dataLen);
    ret=strcmp(ciphe,CIPHE);//相同返回0，不同返回非零
    //发送密码是否匹配标记
    if(!ret){
        passFlag.dataLen=1;
        ret=sendCycle(newfd,(char*)&passFlag,4+passFlag.dataLen);
        ERROR_CHECK(ret,-1,"sendpassFlag1");
        strcpy(NAME,name);//传入传出
        printf("密码正确\n");
    }  
    else{
        passFlag.dataLen=0;
        ret=sendCycle(newfd,(char*)&passFlag,4+passFlag.dataLen);
        ERROR_CHECK(ret,-1,"sendpassFlag0");
        printf("密码错误\n");
        return -1;
    }
    mysql_close(conn);
    return 0;
}

int user_ctr(int newfd,char* name)
{
    printf("用户 %s 登录成功\n",name);
    Train_t getsFlag,putsFlag,mkdirFlag,rmdirFlag,cdFlag,fileList;
    char command[64]={0};
    char mode[32]={0};
    char filename[32]={0};
    char md5_stf[40]={0};
    char buf[1000]={0};
    int dataLen,i,j,procode,ret,dadcode;
    char *spaceFlag;
    //调用数据库
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    char server[20]="localhost";
    char user[20]="root";
    char password[20]="290010";
    char database[20]="net_disk";
    char query[1024]={0};
    conn=mysql_init(NULL);
    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){//连接数据库
        printf("Error connecting to database:%s\n",mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    else{
        printf("Connet database success\n");
    }
    //获取该用户根目录的code值
    sprintf(query,"select * from file where filename='%s'",name);
    ret=mysql_query(conn,query);
    if(ret){
        printf("Error making query:%s\n",mysql_error(conn));
    }
    else{
        res=mysql_use_result(conn);
        if(res){
            if((row=mysql_fetch_row(res))){//如果找到了该用户的根目录
                procode=atoi(row[0]);
                dadcode=atoi(row[1]);
                printf("code = %d , procode = %d\n",procode,dadcode);
            }
        }
        mysql_free_result(res);
    }

    while(1)
    {
        bzero(md5_stf,sizeof(md5_stf));
        bzero(&buf,sizeof(buf));
        bzero(&putsFlag,sizeof(putsFlag));
        bzero(&getsFlag,sizeof(getsFlag));
        bzero(&mkdirFlag,sizeof(mkdirFlag));
        bzero(&rmdirFlag,sizeof(rmdirFlag));
        bzero(&fileList,sizeof(fileList));
        bzero(&cdFlag,sizeof(cdFlag));
        bzero(command,sizeof(command));
        bzero(mode,sizeof(mode));
        bzero(filename,sizeof(filename));
        bzero(query,sizeof(query));
        //接收命令
        recvCycle(newfd,(char*)&dataLen,4);   
        recvCycle(newfd,command,dataLen);
        printf("command : %s , len = %ld\n",command,strlen(command));
        
        //判断是否是带空格的命令,32是空格的ASCII码
        spaceFlag=strchr(command,32);//找不到空格则返回NULL
        if(NULL==spaceFlag){
            //打印当前目录下文件列表
            if(!strcmp(command,"ls")){
                sprintf(query,"select * from file where procode=%d",procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        while((row=mysql_fetch_row(res))){
                            sprintf(buf," - %-15s%s\n",row[2],row[4]);//存进filename,filetype
                            strcat(fileList.buf,buf);
                            bzero(buf,sizeof(buf));
                        }
                        fileList.dataLen=strlen(fileList.buf);
                    }
                    ret=sendCycle(newfd,(char*)&fileList,4+fileList.dataLen);//发送file列表小火车
                    ERROR_CHECK(ret,-1,"sendfilelist");
                    printf("目录列表发送完毕\n");
                    mysql_free_result(res);
                }
            }
            else printf("输入了无效命令\n");
        }
        //下面的命令都是 mode + file/dirname 的格式
        else{
            for(i=0;command[i]!=' ';i++){
                mode[i]=command[i];
            }
            mode[i]='\0';
            for(i++,j=0;command[i]!='\0';i++,j++){
                filename[j]=command[i];
            }
            filename[j]='\0';
            printf("file/dirname : %s\n",filename);

            //创建目录功能
            if(!strcmp(mode,"mkdir")){
                //查询当前目录下是否有重名目录
                sprintf(query,"select * from file where filename='%s' and procode=%d and filetype='dir'",filename,procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果找到了同名目录
                            printf("数据库中当前目录下已有同名目录\n");
                            mysql_free_result(res);
                            mkdirFlag.dataLen=0;
                            ret=sendCycle(newfd,(char*)&mkdirFlag,4+mkdirFlag.dataLen);
                            ERROR_CHECK(ret,-1,"mkdirflag0");
                            continue;
                        }
                        else{//当前目录下没有同名目录
                            printf("该目录名可用\n");
                            mkdirFlag.dataLen=1;
                        }
                    }
                    mysql_free_result(res);
                }
                //传插入命令给数据库
                sprintf(query,"insert into file (procode,filename,belong,filetype) values (%d,'%s','%s','dir')",procode,filename,name);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("目录插入数据库失败\n");
                    mkdirFlag.dataLen=0;
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    printf("新目录 %s 创建成功\n",filename);
                }
                ret=sendCycle(newfd,(char*)&mkdirFlag,4+mkdirFlag.dataLen);
                ERROR_CHECK(ret,-1,"mkdirflag1");                
            }
            
            //删除目录功能
            else if(!strcmp(mode,"rmdir")){
                //查询当前目录下是否存在该目录
                sprintf(query,"select * from file where filename='%s' and procode=%d",filename,procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果存在该目录
                            printf("数据库中当前目录下找到了该目录\n");
                            rmdirFlag.dataLen=1;
                        }
                        else{//当前目录下没有同名目录
                            printf("该目录不存在，删除失败\n");
                            mysql_free_result(res);
                            mkdirFlag.dataLen=0;
                            ret=sendCycle(newfd,(char*)&rmdirFlag,4+rmdirFlag.dataLen);
                            ERROR_CHECK(ret,-1,"mkdirflag0");
                            continue;
                        }
                    }
                    mysql_free_result(res);
                }
                //传删除命令给数据库
                sprintf(query,"delete from file where filename='%s'",filename);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("数据库删除目录失败\n");
                    rmdirFlag.dataLen=0;
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    printf("目录 %s 删除成功\n",filename);
                }
                ret=sendCycle(newfd,(char*)&rmdirFlag,4+rmdirFlag.dataLen);
                ERROR_CHECK(ret,-1,"mkdirflag1");  
            }

            //进入目录功能
            else if(!strcmp(mode,"cd")){
                //如果目录名是..且当前目录非根目录即进入上级目录
                if(!strcmp(filename,"..")){
                    if(dadcode){
                        sprintf(query,"select * from file where code=%d",dadcode);
                        ret=mysql_query(conn,query);
                        if(ret){
                            printf("Error making query:%s\n",mysql_error(conn));
                        }
                        else{
                            res=mysql_use_result(conn);
                            if(res){
                                if((row=mysql_fetch_row(res))){
                                    procode=dadcode;
                                    dadcode=atoi(row[1]);
                                    cdFlag.dataLen=2;
                                    printf("进入上级目录成功\n");
                                }
                                else cdFlag.dataLen=0;
                            }
                        }
                        ERROR_CHECK(ret,-1,"cdflag");
                    }
                    else{
                        printf("尝试进入根目录的上级目录\n");
                        cdFlag.dataLen=3;
                    }
                    ret=sendCycle(newfd,(char*)&cdFlag,4+cdFlag.dataLen);
                    mysql_free_result(res);
                    continue;
                }
                //查询当前目录下是否存在该目录
                sprintf(query,"select * from file where filename='%s' and procode=%d",filename,procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果存在该目录
                            dadcode=procode;
                            procode=atoi(row[0]);
                            printf("目录 %s 存在,code = %d,procode = %d,进入成功\n",filename,procode,dadcode);
                            cdFlag.dataLen=1;
                        }
                        else{//当前目录下没有同名目录
                            printf("该目录不存在，进入失败\n");
                            cdFlag.dataLen=0;
                        }
                        ret=sendCycle(newfd,(char*)&cdFlag,4+cdFlag.dataLen);
                        ERROR_CHECK(ret,-1,"cdflag");
                    }
                    mysql_free_result(res);
                }
            }

            //上传文件功能
            else if(!strcmp(mode,"puts")){
                //若客户端不存在相应文件，则跳出
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(!dataLen)continue;
                //查询当前目录下是否有重名文件
                sprintf(query,"select * from file where filename='%s' and procode=%d and filetype='file'",filename,procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果找到了同名文件
                            printf("数据库中当前目录下已有同名文件\n");
                            putsFlag.dataLen=0;
                            ret=sendCycle(newfd,(char*)&putsFlag,4+putsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"putsflag0");
                            mysql_free_result(res);
                            continue;
                        }
                        else{//当前目录下没有同名文件
                            printf("该文件名可用\n");
                            putsFlag.dataLen=1;
                            ret=sendCycle(newfd,(char*)&putsFlag,4+putsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"putsflag1");
                            mysql_free_result(res);
                        }
                    }
                }
                //获取md5码
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                printf("收到md5码 %s\n",buf);
                //查询该用户所有目录下是否有md5码相同文件
                sprintf(query,"select * from file where md5sum='%s'",buf);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果找到了md5码相同的文件
                            printf("数据库中存在相同文件,不再传输文件内容\n");
                            putsFlag.dataLen=0;
                            ret=sendCycle(newfd,(char*)&putsFlag,4+putsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"putsflag0");
                        }
                        else{//所有目录中没有相同文件
                            printf("没有相同文件,将开始传输文件内容\n");
                            putsFlag.dataLen=1;
                            ret=sendCycle(newfd,(char*)&putsFlag,4+putsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"putsflag1");
                            ret=getFile(newfd);
                            ERROR_CHECK(ret,-1,"getfile");
                            if(0==ret)printf("文件 %s 接收成功\n",filename);
                        }
                    }
                    mysql_free_result(res);
                }
                //在数据库中插入文件信息
                sprintf(query,"insert into file (procode,filename,belong,filetype,md5sum) values (%d,'%s','%s','file','%s')",procode,filename,name,buf);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("文件信息插入数据库失败\n");
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    printf("文件 %s 信息添加成功\n",filename);
                }
            }
            
            //下载文件功能
            else if(!strcmp(mode,"gets")){
                //接收是否正常传文件信息
                recvCycle(newfd,(char*)&dataLen,4);
                recvCycle(newfd,buf,dataLen);
                if(1==dataLen){}//如果是正常传则不做任何事
                else{//如果客户机已存在相同文件名，则传大小过去给他判断不下载或是断点下载

                }
                //查询当前目录下是否存在该文件
                sprintf(query,"select * from file where filename='%s' and procode=%d and filetype='file'",filename,procode);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果存在
                            printf("当前目录下存在该文件\n");
                            getsFlag.dataLen=1;
                            ret=sendCycle(newfd,(char*)&getsFlag,4+getsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"getsflag1");
                            strcpy(buf,row[5]);//保存它的md5码
                            printf("它的md5码是 %s\n",buf);
                            mysql_free_result(res);
                        }
                        else{//当前目录下不存在该文件
                            printf("未找到该文件\n");
                            getsFlag.dataLen=0;
                            ret=sendCycle(newfd,(char*)&getsFlag,4+getsFlag.dataLen);
                            ERROR_CHECK(ret,-1,"getsflag1");
                            mysql_free_result(res);
                            continue;
                        }
                    }
                }
                //找到md5码对应的第一个文件信息即真实文件的文件名
                sprintf(query,"select * from file where filetype='file' and md5sum='%s'",buf);
                ret=mysql_query(conn,query);
                if(ret){
                    printf("Error making query:%s\n",mysql_error(conn));
                }
                else{
                    res=mysql_use_result(conn);
                    if(res){
                        if((row=mysql_fetch_row(res))){//如果找到了md5码相同的文件
                            char FILENAME[32]={0};
                            strcpy(FILENAME,row[2]);
                            printf("找到真实文件 %s ，开始传输文件内容\n",FILENAME);
                            ret=tranFile(newfd,FILENAME);
                            if(0==ret)printf("文件传输成功");
                        }
                        else{
                            printf("所有目录中不存在真实文件,无法开始传输文件内容\n");
                        }
                    }
                    mysql_free_result(res);
                }
            }
        }
    }
}
