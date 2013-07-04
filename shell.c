#include <stdio.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <sys/stat.h> // _stati64()
#include <io.h>




typedef struct FileList{
	char name[20];
	char full_name[_MAX_PATH];
	char maked_data[50];
	__int64 size;
	int checked;
	int is_file;
}FileList;

typedef struct Selected{
	char name[17];
	char full_name[_MAX_PATH];
	struct Selected* link;
}Selected;

__int64 getFileSize(char *filename) {
	struct _stati64 statbuf;

	if ( _stati64(filename, &statbuf) ) return -1; // ���� ���� ���: ���� ������ -1 �� ��ȯ

	return statbuf.st_size;                        // ���� ũ�� ��ȯ
}

int isFileOrDir(char* s) {
	struct _finddatai64_t c_file;
	intptr_t hFile;
	int result;


	if ( (hFile = _findfirsti64(s, &c_file)) == -1L )
		result = -1; // ���� �Ǵ� ���丮�� ������ -1 ��ȯ
	else if (c_file.attrib & _A_SUBDIR)
		result = 0; // ���丮�� 0 ��ȯ
	else
		result = 1; // �׹��� ���� "�����ϴ� ����"�̱⿡ 1 ��ȯ


	_findclose(hFile);

	return result;
}



int get_list(FileList f_list[]){
	struct _finddata_t fd;
	intptr_t handle;
	int f_count=0,i=0;

	if((handle = _findfirst("*.*",&fd))==-1L) return -1;
	else{
		do{
			while(fd.name){
				if(i==20)break;
				if(i<17) f_list[f_count].name[i]=fd.name[i];
				else{
					f_list[f_count].name[17]='.';f_list[f_count].name[18]='.';f_list[f_count].name[19]='\0';//�ڴ� ..���� ����
				}
				i++;
			}i=0;
			strcpy(f_list[f_count].full_name,fd.name);
			f_list[f_count].size=getFileSize(fd.name);
			f_list[f_count].is_file=isFileOrDir(fd.name);
			//f_list[f_count].size=fd.size;
			ctime_s(f_list[f_count].maked_data,sizeof(f_list[f_count].maked_data),&(fd.time_write));
			while(f_list[f_count].maked_data[i+1])i++;//��¥�� �ǳ��� ���๮�ڶ� �η� �ٲ�
			f_list[f_count].maked_data[i]='\0';i=0;
			if(f_list[f_count].checked !=1) f_list[f_count].checked=0; //üũ�Ȱ� �ƴϸ� 0���� �ʱ�ȭ
			f_count++;
		}while(_findnext(handle,&fd)==0);
		_findclose(handle);
	}
	return f_count;
}

void print_f(FileList current_f,int in_detail,Selected* temp){
	int i;
	int dis_size;
	printf("%s",current_f.checked==0?"��":"��");
	printf("%20s",current_f.name);
	printf("%6s",current_f.is_file?" [D]":" [F]");
	if(current_f.size>1000000){//MB�϶�
		dis_size=current_f.size/1000000;
		printf("��%9dM��",dis_size);
	}
	else if(current_f.size>1000){//KB�϶�
		dis_size=current_f.size/1000;
		printf("��%9dK��",dis_size);
	}
	else
		printf("��%10d��",current_f.size);
	printf("%26s��",in_detail==1?current_f.maked_data:"");
	printf("%18s��\n",temp?temp->name:"");
}

void selected_link(Selected** root,FileList f_node){
	Selected* new_node=(Selected*)malloc(sizeof(Selected));//alloction
	Selected* temp=*root;
	int i;
	if(!new_node){
		fprintf(stderr,"�Ҵ����");return;
	}
	if(check_selected(*root,f_node.full_name)){ //�ߺ������ΰ� Ȯ�εǸ� ����
		free(new_node);return; //�Ҵ������ϰ� ����
	}
	for(i=0;f_node.name[i];i++){  //����� ���� �̸� ����
		if(i==16){
			new_node->name[i]='\0';break;
		}
		else if(i>13) new_node->name[i]='.';
		else {
			new_node->name[i]=f_node.name[i]; 
			if(!f_node.name[i+1])new_node->name[i+1]='\0';//������ ������ �� ����
		}
	}
	strcpy(new_node->full_name,f_node.full_name);//Ǯ���� ����
	new_node->link=NULL;
	if(!*root)
		(*root)=new_node;
	else{
		while(temp->link)temp=temp->link;
		temp->link=new_node;
	}
}

int check_selected(Selected* temp,char* full_name){  //�ߺ�Ȯ��
	while(temp){
		if(!strcmp(temp->full_name,full_name))return 1;
		temp=temp->link; 
	}
	return 0;
}

int delete_selected(Selected** root,int number_of_file){
	Selected* temp=*root,*before=*root;
	int i;
	for(i=0;i<number_of_file;i++){
		temp=*root;
		while(temp->link){
			if(!(temp->link->link))before=temp;
			temp=temp->link;//������ ��������.
		}
		rmdir(temp->full_name);
		free(temp);//�����޸� ����
		before->link=NULL;
	}
	*root=NULL;
}


int main(){
	int i=0,j=0,count=0,num=0,down_scroll=0,up_scroll=0,list_end=0,yy=0,xx=0;
	int cusor=0,key,in_detail=0,num_selected_file=0;
	char dir_name[22]="\0";
	FileList f_list[200];
	Selected* root=NULL,*selected_temp=NULL;
	char current_path[_MAX_PATH]="\0";
	while(1){
		count=get_list(f_list);
		system("cls");
		_getcwd(current_path, _MAX_PATH);
		puts("��������������������������");
		puts("��      MONG SHELL      ��");
		fputs("��������������������������",stdout);
		printf("\n");//���⿡ �ð����
		puts("����������������������������������������������������������������������������������������������");
		printf("��%.10s%.90s   ..��\n","path : ",current_path);
		puts("����������������������������������������������������������������������������������������������");
		puts("�����æ�    �����̸�            �� ũ��(kb) ��        ������¥          ��     ���ø��     ��");
		printf("��%s����������������������������������������������������������������������������������������\n",(down_scroll)||(up_scroll)?"���":"����");
		selected_temp=root; //���õȸ���Ʈ ������ ��Ʈ�� �ֽ�ȭ
		for(i=0;i<20;i++){		
			printf("��%s",i==cusor?"��":"��");
			if(count>20){//20���� ��ϸ�����
				if(down_scroll)
					print_f(f_list[i+yy],in_detail,selected_temp);
				else if(up_scroll)
					print_f(f_list[i+xx],in_detail,selected_temp);
				else
					print_f(f_list[i],in_detail,selected_temp);
			}
			else{ //20�� �����϶�
				if(i<count)print_f(f_list[i],in_detail,selected_temp);
				else {
					printf("%2s%26s��%10s��%26s","","","","");
					printf("��%18s��\n",selected_temp?selected_temp->name:"");
				}
			}
			if(selected_temp)//null�� �ƴҶ� �� �������
				selected_temp=selected_temp->link;
		}
		if(!list_end)
			printf("��%s����������������������������������������������������������������������������������������\n",count>19?"���":"����");
		else
			puts("����������������������������������������������������������������������������������������������");

		puts("����������������������������������������������������������������������������������������������");
		printf("������%84s��\n","");
		puts("����������������������������������������������������������������������������������������������");
		puts("��ENTER:����\t\tS:��ϼ���\t\tESC:���� ����\t\t            ��");
		puts("��C:�����Ѹ�� ����\t\tD:�����Ѹ�� ����\tN:��������\t\t            ��");
		puts("��F1:���κ���\t\tF2:���κ���\t\tF3:��������\t\tF4:����������       ��");
		puts("����������������������������������������������������������������������������������������������");
		printf("count: %d cusor: %d  num: %d ����:%s\ncurrent path:%s\n",count,cusor,num,f_list[num].name,current_path);
		printf("up_scroll:%d down_scroll:%d end_list:%d",up_scroll,down_scroll,list_end);
		key=_getch();
		switch(key){
		case 80://key down
			cusor++;num++;
			if(count<21){//20�������϶� ����� Ŀ�� ����
				cusor%=count;
				num=cusor;
			}
			else{
				if(num==count-1)list_end=1;
				if(num==count)num--;
				if(cusor==20){
					cusor=19;//���ڸ��� �ӹ�
					up_scroll=0;
					down_scroll=1;
					yy=num-19;
				}
			}
			break;
		case 72://key up
			cusor--;num--;
			if(count<20){//20�� �����϶� Ŀ������
				if(cusor==-1)cusor=count-1;
				num=cusor;
			}
			else{//���̻��϶�
				if(cusor==-1){
					cusor=0;xx=num;//���ڸ��� �ӹ�
					up_scroll=1;down_scroll=0;list_end=0;
					if(num==-1){
						num=0;up_scroll=0;
					}
				}
			}
			break;
		case 13: //enter
			if(num==1){//����
				_chdir("..");
				_getcwd(current_path, _MAX_PATH);

			}
			else{
				while(current_path[j])j++;
				current_path[j]='\\';current_path[j+1]='\0';j=0;
				if(_chdir(strcat(current_path,f_list[num].full_name)))
					_getcwd(current_path, _MAX_PATH);
			}
			cusor=0;num=0;
			break;
		case 's': //����
			if(num!=0&&num!=1){
				if(num_selected_file<20){ //20�� �����϶��� �߰�����
					selected_link(&root,f_list[num]); //�̾���
					f_list[num].checked=1;
					num_selected_file++;
				}
			}
			break;
		case 'd':
			if(num_selected_file>0){//���õȰ� ��������
				printf("��%84s��\n","������ ��ϵ��� �����Ͻðڽ��ϱ�?(Y/y�Է�)");
				key=_getch();
				if(key=='Y'||key=='y') delete_selected(&root,num_selected_file);
			}
			break;
		case 'n':
			puts("");
			puts("���� �̸��� �Է����ּ���(20�� �̳��ۿ� �ȵǿ�)");
			puts("/\\?|""""<>*: ������ ��� ���������� ���õ˴ϴ�");
			fgets(dir_name,22,stdin);
			for(i=0;dir_name[i];i++);dir_name[i-1]='\0';
			mkdir(dir_name);
			fflush(stdin);
			mkdir("gamgamgam");
			break;
		case 61:
			in_detail=1;
			break;
		case 62:
			in_detail=0;
			break;
		}
	}
}

