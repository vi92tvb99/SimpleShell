#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#define MAXLINE 80

struct redirecting
{
	char command[MAXLINE]; // luu command line truoc khi redirecting
	int tinhChatTapTin; //read only hay write only
};
typedef struct redirecting redirecting;

int tachChuoi(char* command, char** daTach)
{
	char* chuoiCanTach = strdup(command);
	char* token = strtok(chuoiCanTach, " ");	 //tach den gap khoang trang
	daTach[0] = token;
	int i = 1;
	while (token != NULL)
	{
		token = strtok(NULL, " "); //tach tiep theo den khoang trang ke
		daTach[i] = token;
		i++;
	}
	daTach[i] = NULL;
	return i - 1;
}

void themKiTuKetThuc(char* chuoi)
{
	int doDai = strlen(chuoi);
	if (chuoi[doDai - 1] == '\n' || chuoi[doDai - 1] == ' ')
	{
		chuoi[doDai - 1] = '\0';
	}
}



redirecting rCommand(char toanTu, char* command)
{
	int doDaiDongLenh = strlen(command);
	int i;
	redirecting ketQua;
	int tinhChatTapTin;
	char dauToanTu;
	// XAC DINH > HAY <
	if (strstr(command, ">") != NULL)
	{
		dauToanTu = '>';
		tinhChatTapTin = 1;
	}
	else
	{
		dauToanTu = '<';
		tinhChatTapTin = '2';
	}
	int viTri = 0;
	char* lenhThucThi = (char*)malloc(MAXLINE);
	char* tenFile = (char*)malloc(MAXLINE);
	
	//TACH LENH THUC THI VA DAU
	for (i = 0; i < doDaiDongLenh; i++)
	{
		if (command[i] == dauToanTu) //lay tu dau den toan tu~
		{
			if (lenhThucThi[i - 1] == ' ')
			{
				lenhThucThi[i - 1] = '\0';
			}
			else lenhThucThi[i] = '\0';
			viTri = i;
			break;
		}
		lenhThucThi[i] = command[i];
	}
	viTri++;
	while (command[viTri] == ' ') //Bo qua cac khoang trang
	{
		viTri++;
	}

	//COPY TEN FILE SAU > HAY <
	int k = 0;
	for (i = viTri; i < doDaiDongLenh; i++)
	{
		tenFile[k] = command[i];
		k++;
	}
	themKiTuKetThuc(tenFile);

	char* file = strdup(tenFile);
	command = strdup(lenhThucThi);

	//Mo file va set tinh cahat file
	if (dauToanTu == '>') //Set quyen chi ghi
	{
		//xoa file cu tao file moi
		remove(file);
		tinhChatTapTin = open(file, O_WRONLY | O_CREAT, S_IRWXU);
	}
	else //Set quyen chi doc
	{
		tinhChatTapTin = open(file, O_RDONLY | O_CREAT, S_IRWXU);
	}
	strcpy(ketQua.command, command);
	ketQua.tinhChatTapTin = tinhChatTapTin;
	return ketQua;

}




int main()
{
	// PHAN CODE SAN CUA THAY
	char* args[MAXLINE / 2 + 1]; /* command line arguments */
	int shouldrun = 1; /* flag to determine when to exit program */
	char* command = (char*)malloc(MAXLINE * sizeof(char));
	
	// CAU LENH LICH SU
	char* cauLenhTruoc = (char*)malloc(MAXLINE * sizeof(char));
	cauLenhTruoc = NULL;

	// LENH TRONG PIPE
	char* commandTruoc = NULL; //Tao chuoi de luu lenh trong pipe
	char* commandSau = NULL;

	// DUNG TRONG REDIRECTING
	int huongRedirect = 0; /* 0. Khong co - 1. > - 2. <   */
	int tinhChatTapTin = 1; //dung trong redirect


	while (shouldrun)
	{
		int lichSu = 0; //ktra neu xuat No commands in history
		printf("osh> ");
		fflush(stdin);
		fflush(stdout);
		fgets(command, 1000, stdin);
		themKiTuKetThuc(command);


		// ====================== LUU LICH SU ===========================
		if (strcmp(command, "!!") == 0) //Co yeu cau !!
		{
			if (cauLenhTruoc == NULL) //neu khong co cau lenh truoc
			{
				printf("No commands in history.\n");
				lichSu = 1;
			}
			else //neu co cau lenh truoc
			{
				printf("%s\n", cauLenhTruoc);
				command = strdup(cauLenhTruoc);
			}
		}
		else //khong co yeu cau !! thi luu cau lenh
		{
			cauLenhTruoc = strdup(command);
		}
		// ====================== KIEM TRA XEM CO & KHONG ===========================
		int kiemTra = 0;
		if (strstr(command, "&") != NULL)
		{
			kiemTra = 1;
		}

		// ====================== COPY CHUOI COMMAND DE XU LY =======================
		char* commandCopy;
		commandCopy = (char*)malloc(MAXLINE * sizeof(char));
		if (kiemTra == 1)
		{
			int i;

			for (i = 0; i <= strlen(command); i++)
			{
				commandCopy[i] = command[i];
			}
			commandCopy[strlen(command) - 1] = '\0';
		}
		else
		{
			if (commandCopy != NULL)
			{
				memset(commandCopy, '\0', strlen(commandCopy));
			}
		}

		//======================= REDIRECTING ============================

		if (strstr(command, ">") != NULL || strstr(command, "<") != NULL) //co yeu cau redirect
		{
			int doDaiCmd = strlen(command);
			char tempCmd[MAXLINE];
			strcpy(tempCmd, command);
			char dauToanTu;

			if (strstr(command, ">") != NULL)
			{
				huongRedirect = 1;
				dauToanTu = '>';
			}
			else
			{
				huongRedirect = 2;
				dauToanTu = '<';
			}
			//Luu de xu li trong fork()
			redirecting temp;
			temp = rCommand(dauToanTu, tempCmd);
			command = strdup(temp.command);
			tinhChatTapTin = temp.tinhChatTapTin;
		}

		//======================= PIPE VA THUC HIEN LENH ============================

		int kiemTraPipe = 0; //1 la co, 0 la khong

		if (lichSu == 0)
		{

			if (strstr(command, "|") != NULL) //co su dung pipe
			{
				kiemTraPipe = 1;
				commandTruoc = (char*)malloc(MAXLINE);
				commandSau = (char*)malloc(MAXLINE);
				int doDaiCmd = strlen(command);
				int i = 0;
				int viTri = 0;
				for (i = 0; i < doDaiCmd; i++)
				{
					if (commandTruoc[i] == '|')
					{
						commandTruoc[i] = '\0';
						viTri = i + 1;
						break;
					}
					commandTruoc[i] = command[i];
				}
				while (command[viTri] == ' ')
				{
					viTri++;
				}
				int chayViTri = 0;
				for (i = viTri; i < doDaiCmd; i++)
				{
					commandSau[chayViTri] = command[i];
					chayViTri++;
				}
				commandSau[chayViTri] = '\0';
			}

			// ========= THOAT NEU NHAP EXIT KHONG THI TIEP TUC THUC HIEN LENH ===========

			if (strcmp(command, "\0") == 0)
			{
				fflush(stdin);
			}
			if (strcmp(command, "exit") == 0)
			{
				shouldrun = 0;
			}
			else
			{
				int soChuoiDaTach = 0;
				if (kiemTra == 1)
				{
					soChuoiDaTach = tachChuoi(commandCopy, args);
				}
				else soChuoiDaTach = tachChuoi(command, args);
				pid_t pidCon = fork();

				if (pidCon == 0) //Neu dang la Process con
				{
					if (huongRedirect == 1) // theo huong >
					{
						dup2(tinhChatTapTin, 1);
					}
					else
					{
						if (huongRedirect == 2) // theo huong <
						{
							dup2(tinhChatTapTin, 0);
						}
					}

					if (kiemTraPipe == 1) //co pipe
					{
						int check[2]; //[0] la READ_END, [1] la WRITE_END
						pipe(check); //pipe
						pid_t pidChau = fork(); //tao fork cho process con
						if (pidChau == 0) //process chau 1
						{
							dup2(check[1], 1); //truyen output WRITE_END
							close(check[0]);
							close(check[1]);
							tachChuoi(commandTruoc, args);
							if (execvp(args[0], args) == -1)
							{
								printf("Error executing command!\n");
								exit(0);
							}
							else exit(0);

						}
						else
						{
							if (pidChau > 0) //process chau 2
							{
								dup2(check[0], 0); //doc tu READ_END
								close(check[0]);
								close(check[1]);
								tachChuoi(commandSau, args);
								if (execvp(args[0], args) == -1)
								{
									printf("Error executing command!\n");
									exit(0);
								}
								else exit(0);
							}
							else
							{
								printf("Error executing pipe command!\n");
							}
							exit(0);
						}
					}

					if (execvp(args[0], args) == -1)
					{
						printf("Error executing command!\n");
						close(tinhChatTapTin);
						exit(0);
					}
					else
					{
						close(tinhChatTapTin);
						exit(0);
					}

				}
				else
				{
					if (pidCon > 0)
					{
						wait(NULL);
						if (kiemTra == 1)
						{

							continue;
						}
						else wait(&pidCon);
					}
					else
					{
						printf("Error creating process!\n");
					}


				}
			}
		}
	}
	free(command);
	free(cauLenhTruoc);
	free(commandTruoc);
	free(commandSau);
	return 0;
}

