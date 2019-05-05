#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "jrb.h"
#include "btree.h"

GtkWidget *window;
GtkWidget *frame;
GtkWidget *about_dialog;
GtkWidget *textView;
GtkWidget *textSearch;
GtkListStore *list;
BTA * data = NULL;
BTA *soundexTree=NULL;
static char code[128] = { 0 };

const char* soundex(const char *s)
{
	static char out[5];
	int c, prev, i;

	out[0] = out[4] = 0;
	if (!s || !*s) return out;

	out[0] = *s++;

	/* first letter, though not coded, can still affect next letter: Pfister */
	prev = code[(int)out[0]];
	for (i = 1; *s && i < 4; s++) {
		if ((c = code[(int) * s]) == prev) continue;

		if (c == -1) prev = 0;	/* vowel as separator */
		else if (c > 0) {
			out[i++] = c + '0';
			prev = c;
		}
	}
	while (i < 4) out[i++] = '0';
	return out;
}

void add_code(const char *s, int c)
{
	while (*s) {
		code[(int)*s] = code[0x20 ^ (int) * s] = c;
		s++;
	}
}

void init()
{
	static const char *cls[] =
	{ "AEIOU", "", "BFPV", "CGJKQSXZ", "DT", "L", "MN", "R", 0};
	int i;
	for (i = 0; cls[i]; i++)
		add_code(cls[i], i - 1);
}


void set_size(GtkWidget * gw, int width, int height) {
	gtk_widget_set_size_request(gw, width, height);
}

//them tien ich vao vung chua GtkFixed tai vi tri da cho
//x: vi tri nam ngang, y: doc
void set_pos(GtkWidget * gw, int x, int y) {
	gtk_fixed_put(GTK_FIXED(frame), gw, x, y);
}

void make_about_dialog() {
	about_dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Từ điển Anh-Việt");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), "1.4");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), "2018 - 05 Nhom 5");
	
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), "Dang Thi Hoa 20161600\n"
	                              " Bui Phuong Thao - ********\n"
	                              "Do Dinh Hoang - 20166135 .");
	gtk_about_dialog_set_logo_icon_name(GTK_ABOUT_DIALOG(about_dialog), NULL);

}
void set_textView_text(char * text) {
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView));
	if (buffer == NULL) {
		printf("Get buffer fail!");
		buffer = gtk_text_buffer_new(NULL);
	}
	gtk_text_buffer_set_text(buffer, text, -1);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(textView), buffer);
}
void find_word(GtkWidget * widget, gpointer No_need) {
	int rsize;
	char name[50];
	char mean[5000];
	strcpy(name, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if (btsel(data,name,mean,500*sizeof(char),&rsize))
		set_textView_text("\nRất tiếc, từ này hiện không có trong từ điển."
		                  "\n\nGợi ý:\t-Nhấn tab để tìm từ gần giống từ vừa nhập!"
		                  "\n\t\t-Hoặc nhấn nút \"Thêm từ\", để bổ sung vào từ điển.");
	else
		set_textView_text(mean);
}
int check_prefix(char * str1,char * str2) {
	int slen2 = strlen(str2);
	int slen1 = strlen(str1);
	int i;
	if (slen1 < slen2)
		return 0;
	for (i = 0; i < slen2; i++)
		if (str1[i] != str2[i])
			return 0;
	return 1;
}
void find_Soundex(char *word,char *result)
{
	char soundexword[5],name[50],soundex1[5];
	strcpy(soundexword,soundex(word));
	int i=0,rsize;
	BTint value;
	btsel(soundexTree,"",soundex1,5*sizeof(char),&rsize);
	
	while(btseln(soundexTree,name,soundex1,5*sizeof(char),&rsize)==0 && i<10){
		if(strcmp(soundexword,soundex1)==0&&strcmp(word,name)!=0)
		{
			//printf("%s\n",soundex1 );
			strcat(result,name);
			strcat(result, "\n");
			i++;
		}
		
	}
}
void add_to_list(JRB nextword, int number) {
	GtkTreeIter Iter;
	JRB tmp;
	jrb_traverse(tmp, nextword) {
			gtk_list_store_append(list, &Iter);
			gtk_list_store_set(list, &Iter, 0, jval_s(tmp->key), -1 );
			if (number-- < 1)
				return;
		}
	
}


void suggest(char * word, gboolean Tab_pressed) {
	
	char temp[100],name[50],mean[1000],soundex[5];
	int i,j=0,rsize;
	int max;
	GtkTreeIter Iter;
	JRB tmp, nextword;
	int check=0;
	strcpy(temp, word);
	nextword=make_jrb();
	gtk_list_store_clear(list);
	if (btsel(data,word,mean,500*sizeof(char),&rsize) ==  0) {
		check = 1;
		gtk_list_store_append(list, &Iter); 
		gtk_list_store_set(list, &Iter, 0, word, -1 );
	}
	if (check==0)
		btins(data, temp, "", sizeof(char));
		btsel(data,word,mean,500*sizeof(char),&rsize) ;
	for (i = 0; i < 100; i++) {
		if(btseln(data,temp,mean,500*sizeof(char),&rsize)==0)
		{
			if (check_prefix(temp, word)) {//neu word la tiep dau ngu cua next word thi add next word vao cay jrb
				jrb_insert_str(nextword, strdup(temp), JNULL);
			}
			else break;//khong thi break
		}
	}
	
	if (check==0&& Tab_pressed==TRUE) {
		if (jrb_empty(nextword)) {
			char result[1000] = "Gợi ý:\n";
			find_Soundex(word,result);
			set_textView_text(result);
		}
		else {//autocomplete
			strcpy(temp, jval_s(jrb_first(nextword)->key));
			gtk_entry_set_text(GTK_ENTRY(textSearch),temp);
			gtk_editable_set_position(GTK_EDITABLE(textSearch), strlen(temp));
		}

	}
	else
	{
		add_to_list(nextword, 15);
		jrb_free_tree(nextword);
		
	}
	if(check==0)
	{
		btdel(data, word);
	}
	
}


gboolean update_word(GtkWidget * entry, GdkEvent * event, gpointer No_need) {//nhap va cap nhat tu trong text Search
	GdkEventKey *keyEvent = (GdkEventKey *)event;
	char word[50];
	int len;
	strcpy(word, gtk_entry_get_text(GTK_ENTRY(textSearch)));
	if (keyEvent->keyval == GDK_KEY_Tab) {
		suggest(word,  TRUE);
	}
	else {
		if (keyEvent->keyval != GDK_KEY_BackSpace) {
			len = strlen(word);
			word[len] = keyEvent->keyval;
			word[len + 1] = '\0';
		}
		else {
			len = strlen(word);
			word[len - 1] = '\0';
		}
		suggest(word, FALSE);
	}
	return FALSE;
}
void Show_about_dialog(GtkWidget * widget, gpointer dialog) {
	make_about_dialog();	
	gtk_dialog_run(GTK_DIALOG(about_dialog));
	gtk_widget_destroy (about_dialog);
}
void Show_message(GtkWidget * parent , GtkMessageType type,  char * mms, char * content) {
	GtkWidget *mdialog;
	mdialog = gtk_message_dialog_new(GTK_WINDOW(parent),
	                                 GTK_DIALOG_DESTROY_WITH_PARENT,
	                                 type,
	                                 GTK_BUTTONS_OK,
	                                 "%s", mms);
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(mdialog), "%s",  content);
	gtk_dialog_run(GTK_DIALOG(mdialog));
	gtk_widget_destroy(mdialog);
}
void Add_word_to_dict(GtkWidget * widget, gpointer Array) {
	GtkWidget* inputtext = ((GtkWidget**)Array)[0];
	GtkWidget* mean = ((GtkWidget**)Array)[1];
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	BTint x;
	int result;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);
	gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);
	char * wordtext = (char*)gtk_entry_get_text(GTK_ENTRY(inputtext));
	char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);
	if (wordtext[0] == '\0' || meantext[0] == '\0')
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_WARNING, "Cảnh báo!", "Không được bỏ trống phần nào.");
	else if (bfndky(data, wordtext, &x ) == 0)
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Từ vừa nhập đã có trong từ điển.");
	else
	{
		
		result = btins(data, wordtext, meantext, strlen(meantext) + 1);
		
		if ( result == 0)
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_INFO, "Thành công!", "Đã thêm từ vừa nhập vào từ điển.");
		else
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	}
}
void Edit_word_in_dict(GtkWidget * widget, gpointer Array) {
	GtkWidget* inputtext = ((GtkWidget**)Array)[0];
	GtkWidget* mean = ((GtkWidget**)Array)[1];
	GtkTextIter st_iter;
	GtkTextIter ed_iter;
	BTint x;
	int result;
	gtk_text_buffer_get_start_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter);//Lay chi so dau buffer
	gtk_text_buffer_get_end_iter (gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &ed_iter);//Lay chi so cuoi buffer
	char * wordtext = (char*)gtk_entry_get_text(GTK_ENTRY(inputtext));//Lay noi dung o nhap tu tieng anh
	char * meantext =  gtk_text_buffer_get_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(mean)), &st_iter, &ed_iter, FALSE);// Lay toan bo text trong Textview(cho hienthi nghia)
	if (wordtext[0] == '\0' || meantext[0] == '\0') // Bo trong thi canh bao
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_WARNING, "Cảnh báo!",
		             "Không được bỏ trống phần nào.");
	else if (bfndky(data, wordtext, &x ) != 0)//Neu tim thay canh bao
		Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!",
		             "Không tìm thấy từ này trong từ điển.");
	else {
	
		result = btupd(data, wordtext, meantext, strlen(meantext) + 1);//Cap nhat la word va nghia
		
		if (result == 0)
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_INFO, "Thành công!",
			             "Đã cập nhật lại nghĩa của từ trong từ điển.");// Cap nhat thanh cong
		else//Cap nhat that bai
			Show_message(((GtkWidget**)Array)[2], GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	}
}
void Delete_word_from_dict(char * word) {
	int result = btdel(data, word);

	char anu[100] = "Đã xóa từ ";
	if (result == 0)
		Show_message(window, GTK_MESSAGE_INFO, "Thành công!", strcat(strcat(anu, word), " khỏi từ điển"));
	else
		Show_message(window, GTK_MESSAGE_ERROR, "Xảy ra lỗi!", "Có lỗi bất ngờ xảy ra.");
	set_textView_text("");
	gtk_entry_set_text(GTK_ENTRY(textSearch), "");
	gtk_widget_grab_focus(textSearch);
}
void destroy_something(GtkWidget * widget, gpointer gp) {
	gtk_widget_destroy(gp);
}
void Show_add_dialog(GtkWidget * widget, gpointer dialog) {
	GtkWidget *adddialog;
	adddialog = gtk_dialog_new_with_buttons("Thêm từ", GTK_WINDOW(window),
	                                        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget* tframe = gtk_frame_new("Từ vựng:");
	GtkWidget* bframe = gtk_frame_new("Nghĩa:");
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * OkButton =  gtk_button_new_with_label("Thêm");
	GtkWidget * CancelButton = gtk_button_new_with_label("Hủy");
	GtkWidget* inputtext = gtk_entry_new();
	GtkWidget* mean = gtk_text_view_new();
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);

	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	set_size(tframe, 300, 50);
	set_size(bframe, 300, 200);
	set_size(box, 300, 50);
	set_size(OkButton, 100, 40);
	set_size(CancelButton, 100, 40);
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);//Chong tran be ngang
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(adddialog))), dialog_ground,  TRUE, TRUE, 0);
	GtkWidget * data_array[3];
	data_array[0] = inputtext;
	data_array[1] = mean;
	data_array[2] = adddialog;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(Add_word_to_dict), data_array);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_something), adddialog);
	gtk_widget_show_all(adddialog);
	gtk_dialog_run(GTK_DIALOG(adddialog));
	gtk_widget_destroy(adddialog);
}
void Show_edit_dialog(GtkWidget * widget, gpointer dialog) {
	BTint x;
	if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
	        bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
		return;
	}
	find_word(NULL, NULL);
	GtkWidget *editdialog;
	editdialog = gtk_dialog_new_with_buttons("Sửa từ", GTK_WINDOW(window),
	             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, NULL, NULL);

	GtkWidget *dialog_ground = gtk_fixed_new();
	GtkWidget* tframe = gtk_frame_new("Từ vựng:");
	GtkWidget* bframe = gtk_frame_new("Nghĩa:");
	GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget * OkButton =  gtk_button_new_with_label("Lưu");
	GtkWidget * CancelButton = gtk_button_new_with_label("Hủy");
	GtkWidget* inputtext = gtk_search_entry_new();
	GtkWidget* mean = gtk_text_view_new();
	GtkWidget* scroll = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(box), OkButton, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(box), CancelButton, TRUE, TRUE, 2);
	set_size(tframe, 300, 50);
	set_size(bframe, 300, 200);
	set_size(box, 300, 50);
	set_size(OkButton, 100, 40);
	set_size(CancelButton, 100, 40);
	gtk_widget_set_margin_left(inputtext, 2);
	gtk_widget_set_margin_right(inputtext, 2);
	gtk_fixed_put(GTK_FIXED(dialog_ground), tframe, 0, 0);
	gtk_fixed_put(GTK_FIXED(dialog_ground), bframe, 0, 55);
	gtk_fixed_put(GTK_FIXED(dialog_ground), box, 0, 260);
	gtk_container_add(GTK_CONTAINER(tframe), inputtext);
	gtk_container_add(GTK_CONTAINER(scroll), mean);
	gtk_container_add(GTK_CONTAINER(bframe), scroll);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(mean), GTK_WRAP_WORD_CHAR);//Chong tran be ngang
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(editdialog))), dialog_ground,  TRUE, TRUE, 0);
	gtk_widget_set_sensitive(tframe, FALSE);
	gtk_entry_set_text(GTK_ENTRY(inputtext), gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(mean), gtk_text_view_get_buffer(GTK_TEXT_VIEW(textView)));
	GtkWidget * data_array[3];
	data_array[0] = inputtext;
	data_array[1] = mean;
	data_array[2] = editdialog;
	g_signal_connect(OkButton, "clicked", G_CALLBACK(Edit_word_in_dict), data_array);
	g_signal_connect(CancelButton, "clicked", G_CALLBACK(destroy_something), editdialog);
	gtk_widget_show_all(editdialog);
	gtk_dialog_run(GTK_DIALOG(editdialog));
	gtk_widget_destroy(editdialog);
}

void Show_delete_dialog(GtkWidget * widget, gpointer dialog) {
	BTint x;
	if (gtk_entry_get_text(GTK_ENTRY(textSearch))[0] == 0 ||
	        bfndky(data, (char*)gtk_entry_get_text(GTK_ENTRY(textSearch)), &x) != 0) {
		Show_message(window, GTK_MESSAGE_WARNING, "Cảnh báo:", "Từ vừa nhập không có trong từ điển!");
		return;
	}
	GtkWidget *deldialog;
	deldialog = gtk_message_dialog_new(GTK_WINDOW(window),
	                                   GTK_DIALOG_DESTROY_WITH_PARENT,
	                                   GTK_MESSAGE_QUESTION,
	                                   GTK_BUTTONS_YES_NO,
	                                   "Xóa: \"%s\"?", gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(deldialog), "Bạn thực sự muốn xóa từ \"%s\" chứ?",
	        gtk_entry_get_text(GTK_ENTRY(textSearch)));

	int result = gtk_dialog_run(GTK_DIALOG(deldialog));
	if (result == GTK_RESPONSE_YES)
		Delete_word_from_dict((char*)gtk_entry_get_text(GTK_ENTRY(textSearch)));
	gtk_widget_destroy(deldialog);
}


int main(int argc, char** argv) {

	 data = btopn("AnhViet.dat",0,1);
	soundexTree=btopn("soundexTree.dat",0,1);
	GtkWidget *groupBox;
	GtkWidget *inputBox;
	GtkWidget *outputBox;
	GtkWidget *toolbar;
	
	GtkWidget *addButton;
	GtkWidget *delButton;
	GtkWidget *editButton;
	GtkWidget *infoButton;
	GtkWidget *searchButton;
	GtkWidget *scrolling;

	
	GtkEntryCompletion *comple;

	

	gtk_init(&argc, &argv);
	init();  //khoi dong soundex
	
	//khoi tao cua so
	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_title(GTK_WINDOW(window),"Dictionary");
	gtk_window_set_resizable(GTK_WINDOW(window),FALSE);
	
	// khoi tao nen
	frame = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(window),frame);
	gtk_widget_set_margin_left(frame, 5);
	gtk_widget_set_margin_right(frame, 5);
	gtk_widget_set_margin_top(frame,5);
	gtk_widget_set_margin_bottom(frame, 15);
	//
	
	//toolbar
	toolbar = gtk_toolbar_new();
	gtk_toolbar_set_style(GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH);
	gtk_widget_set_size_request(toolbar,560,40);
	gtk_fixed_put(GTK_FIXED(frame),toolbar,0,0);
	addButton = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	gtk_tool_button_set_label(addButton,"Add");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),addButton,-1);
	delButton = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
	gtk_tool_button_set_label(delButton,"Delete");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),delButton,-1);
	editButton = gtk_tool_button_new_from_stock(GTK_STOCK_EDIT);
	gtk_tool_button_set_label(editButton,"Edit");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),editButton,-1);
	infoButton = gtk_tool_button_new_from_stock(GTK_STOCK_DIALOG_INFO);
	gtk_tool_button_set_label(infoButton,"Information");
	gtk_toolbar_insert(GTK_TOOLBAR(toolbar),infoButton,-1);

	//Khoi tao GroupBox
	groupBox = gtk_frame_new("Nhập từ:");
	set_size(groupBox, 600, 50);
	set_pos(groupBox, 5, 65);

	//inputBox
	inputBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	gtk_container_add(GTK_CONTAINER(groupBox), inputBox);
	
	textSearch = gtk_search_entry_new();
	set_size(textSearch, 490, 20);
	gtk_box_pack_start(GTK_BOX(inputBox),textSearch,TRUE,TRUE,5);

	searchButton = gtk_button_new_with_label("Tìm kiếm");
	gtk_widget_set_margin_bottom(searchButton, 10);
	gtk_widget_set_margin_top(searchButton, 10);
	gtk_box_pack_start(GTK_BOX(inputBox),searchButton,TRUE,TRUE,5);
	//auto complete
	comple = gtk_entry_completion_new();
	gtk_entry_completion_set_text_column(comple, 0);
	list = gtk_list_store_new(15, G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING,
	                          G_TYPE_STRING, G_TYPE_STRING);

	gtk_entry_completion_set_model(comple, GTK_TREE_MODEL(list));
	gtk_entry_set_completion(GTK_ENTRY(textSearch), comple);

	//outputBox

	outputBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	set_size(outputBox, 595, 400);
	set_pos(outputBox, 5, 140);
	
	textView = gtk_text_view_new();
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textView), GTK_WRAP_WORD_CHAR);//Chong tran be ngang

	//Khoi tao thanh keo truot cho textView
	scrolling = gtk_scrolled_window_new(NULL, NULL);
	gtk_container_add(GTK_CONTAINER(scrolling), textView);
	gtk_box_pack_start(GTK_BOX(outputBox), scrolling, TRUE, TRUE, 2);

	gtk_widget_show_all(window);

	g_signal_connect(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);//Ket thuc chuong trinh khi dong cua so chinh

	//Gan su kien cho cac nut
	g_signal_connect(searchButton, "clicked", G_CALLBACK(find_word), NULL);
	g_signal_connect(addButton, "clicked", G_CALLBACK(Show_add_dialog), NULL);
	g_signal_connect(editButton, "clicked", G_CALLBACK(Show_edit_dialog), NULL);
	g_signal_connect(delButton, "clicked", G_CALLBACK(Show_delete_dialog), NULL);
	g_signal_connect(infoButton, "clicked", G_CALLBACK(Show_about_dialog), about_dialog);
	g_signal_connect(textSearch, "key-press-event", G_CALLBACK(update_word), NULL);
	g_signal_connect(textSearch, "activate", G_CALLBACK(find_word), NULL);

	gtk_main();



	btcls(data);
	return 0;
}



	
	
	

	
