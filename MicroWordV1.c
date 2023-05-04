#include <stdio.h>
#include <string.h>
#include <stdlib.h>
void Clean(){
    //system("cls");
    system("clear");
    //printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    //printf("\e[1;1H\e[2J");
}
void Signal(){
    printf("\n--------------> ");
}
void Lines(int a){
    int i = 0;
    printf("\n");
    for (i=0;i<a;i++){
        printf("*****");
    }
    printf("\n");
}
void Pause(int b){
    char Wait;
    printf("\n\tPresiona ENTER 2 veces para continuar\n\n");
    Lines(b);
    scanf("%c%*c",&Wait);
    Clean();
}

void invalidOpt(){
    Clean();
    Lines(13);
    printf("\n\t¡Quedaste! La opcion seleccionada no es válida, por favor ingresa un valor válido\n\n");
    Pause(13);
}
void creadorDocs(char* titleName);
void elimArch(char* titleName);
int main() {
    int opt =5;
    char titleName[255];
    char* extension =".docx";
    do{
        Clean();
        Lines(14);
        printf("\n\t\t\t\t\t\t\tby. Emi\n\n\tXML microWord");
        printf("\n\tOpciones:\n\n\t\t\t1.- Crear Documento\n\t\t\t2.- Reescribir documento\n\t\t\t3.- Eliminar documento\n\t\t\t0.- Salir\n");
        Lines(14);
        Signal();
        scanf("%d%*c",&opt);
        Clean();
        if (opt != 0){
           Lines(14);
            printf("\n\tPor favor ingresa el nombre del documento\n");
            Lines(14);
            Signal();
            scanf("%[^\n]%*c",titleName);
            strcat(titleName,extension);
        }

        switch(opt){

            case 1: Clean();
                    if (fopen(titleName, "r") != NULL) {
                        Lines(5);
                        printf("\n\tError: El archivo ya existe!\n");
                        Pause(5);
                        break;
                    }
                    Lines(13);
                    printf("\n\tCreando documento %s... \n",titleName);
                    creadorDocs(titleName);
                    break;

            case 2: Clean();
                    if (fopen(titleName, "r") != NULL) {
                        Lines(13);
                        printf("\n\tReescribiendo documento %s... \n",titleName);
                        creadorDocs(titleName);
                        break;
                    }
                    Lines(5);
                    printf("\n\tError: El archivo No existe!\n");
                    Pause(5);
                    break;

            case 3: Clean();
                    Lines(13);
                    printf("\n\tEliminando documento%s... \n",titleName);
                    elimArch(titleName);
                    Pause(13);
                    break;

            case 0: Clean();
                    Lines(13);
                    printf("\n\tSaliendo...\n");
                    Pause(13);
                    return 0;
            default: invalidOpt();
                     break;
        }
    }while(opt!=0);
    // Check if the file already exists


    return 0;
}
void creadorDocs(char* titleName){
    Clean();
    // Create the file pointer
    FILE *file;
    // Open the file for writing
    file = fopen(titleName, "w");

    // Write the file header
    char* header1 = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<?mso-application progid=\"Word.Document\"?><w:wordDocument xmlns:w=\"http://schemas.microsoft.com/office/word/2003/wordml\" xmlns:wx=\"http://schemas.microsoft.com/office/word/2003/auxHint\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:aml=\"http://schemas.microsoft.com/aml/2001/core\" xmlns:dt=\"uuid:C2F41010-65B3-11d1-A29F-00AA00C14882\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:number=\"urn:oasis:names:tc:opendocument:xmlns:datastyle:1.0\" xml:space=\"preserve\" w:embeddedObjPresent=\"no\">";

    char* header2="\n\t\t<o:DocumentProperties xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><o:Title/><o:Subject/><o:Keywords/><o:Description/><o:Category/><o:Author/><o:LastAuthor/><o:Manager/><o:Company/><o:HyperlinkBase/><o:Revision>1</o:Revision><o:TotalTime>2</o:TotalTime><o:LastPrinted/><o:Created/><o:LastSaved>2023-04-22T10:17:54.604616670Z</o:LastSaved><o:Pages>1</o:Pages><o:Words>52</o:Words><o:Characters>306</o:Characters><o:Paragraphs>3</o:Paragraphs></o:DocumentProperties><o:CustomDocumentProperties xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><o:Editor dt:dt=\"string\">LibreOffice/7.5.0.2$Linux_X86_64 LibreOffice_project/c0dd1bc3f1a385d110b88e26ece634da94921f58</o:Editor><o:Language dt:dt=\"string\"/></o:CustomDocumentProperties>\n<w:fonts><w:defaultFonts w:ascii=\"\" w:h-ansi=\"\" w:fareast=\"\" w:cs=\"\"/><w:font w:name=\"Liberation Sans\"><w:family w:val=\"Swiss\"/><w:pitch w:val=\"variable\"/></w:font><w:font w:name=\"Liberation Serif\"><w:family w:val=\"Roman\"/><w:pitch w:val=\"variable\"/></w:font><w:font w:name=\"Noto Sans CJK SC\"><w:family w:val=\"System\"/><w:pitch w:val=\"variable\"/></w:font><w:font w:name=\"Noto Sans Devanagari\"><w:family w:val=\"Swiss\"/><w:pitch w:val=\"\"/></w:font><w:font w:name=\"Noto Sans Devanagari1\"><w:family w:val=\"System\"/><w:pitch w:val=\"variable\"/></w:font><w:font w:name=\"Noto Serif CJK SC\"><w:family w:val=\"System\"/><w:pitch w:val=\"variable\"/></w:font></w:fonts>";

    char* header3 ="\n\t\t\t<w:lists xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:listDef w:listDefId=\"0\"><w:lvl w:ilvl=\"0\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%1\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"1\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%2\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"2\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%3\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"3\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%4\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"4\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%5\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"5\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%6\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"6\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%7\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"7\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%8\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl><w:lvl w:ilvl=\"8\"><w:start w:val=\"1\"/><w:nfc w:val=\"255\"/><w:lvlText w:val=\"%%9\"/><w:lvlJc w:val=\"left\"/><w:suff w:val=\"Nothing\"/></w:lvl></w:listDef><w:list w:ilfo=\"1\"><w:ilst w:val=\"0\"/></w:list></w:lists>\n<w:styles>";

    char* header4="<w:style w:styleId=\"default-paragraph-style\" w:type=\"paragraph\" w:default=\"on\"><w:name w:val=\"default-paragraph-style\"/><w:pPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:adjustRightInd w:val=\"off\"/><w:spacing/><w:ind/><w:widowControl w:val=\"off\"/><w:pBdr/><w:ind/></w:pPr><w:rPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:rFonts w:ascii=\"Liberation Serif\" w:h-ansi=\"Liberation Serif\" w:fareast=\"Noto Serif CJK SC\" w:cs=\"Noto Sans Devanagari1\"/><w:sz w:val=\"24\"/><w:lang w:val=\"en-US\"/></w:rPr></w:style><w:style w:styleId=\"default-table-style\" w:type=\"table\" w:default=\"on\"><w:name w:val=\"default-table-style\"/><w:tblPr><w:tblInd xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:w=\"0\" w:type=\"auto\"/></w:tblPr></w:style><w:style w:styleId=\"Standard\" w:type=\"paragraph\"><w:basedOn w:val=\"default-paragraph-style\"/><w:name w:val=\"Standard\"/></w:style><w:style w:styleId=\"Heading\" w:type=\"paragraph\"><w:basedOn w:val=\"Standard\"/><w:name w:val=\"Heading\"/><w:next w:val=\"Text_20_body\"/><w:pPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:adjustRightInd w:val=\"off\"/><w:spacing w:before=\"239.79\" w:after=\"120.255\"/><w:ind/><w:widowControl w:val=\"off\"/><w:pBdr/><w:ind/></w:pPr><w:rPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:rFonts w:ascii=\"Liberation Sans\" w:h-ansi=\"Liberation Sans\" w:fareast=\"Noto Sans CJK SC\" w:cs=\"Noto Sans Devanagari1\"/><w:sz w:val=\"28\"/></w:rPr></w:style><w:style w:styleId=\"Text_20_body\" w:type=\"paragraph\"><w:basedOn w:val=\"Standard\"/><w:name w:val=\"Text_20_body\"/><w:pPr>xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:adjustRightInd w:val=\"off\"/><w:spacing w:line-rule=\"auto\" w:line=\"276\" w:before=\"0\" w:after=\"139.9855\"/><w:ind/><w:widowControl w:val=\"off\"/><w:pBdr/><w:ind/></w:pPr></w:style><w:style w:styleId=\"List\" w:type=\"paragraph\"><w:basedOn w:val=\"Text_20_body\"/><w:name w:val=\"List\"/><w:rPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:rFonts w:cs=\"Noto Sans Devanagari\"/></w:rPr></w:style><w:style w:styleId=\"Caption\" w:type=\"paragraph\"><w:basedOn w:val=\"Standard\"/><w:name w:val=\"Caption\"/><w:pPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:adjustRightInd w:val=\"off\"/><w:spacing w:before=\"120.255\" w:after=\"120.255\"/><w:ind/><w:widowControl w:val=\"off\"/><w:suppressLineNumbers/><w:pBdr/><w:ind/></w:pPr><w:rPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:rFonts w:cs=\"Noto Sans Devanagari\"/><w:sz w:val=\"24\"/><w:i/></w:rPr></w:style><w:style w:styleId=\"Index\" w:type=\"paragraph\"><w:basedOn w:val=\"Standard\"/><w:name w:val=\"Index\"/><w:pPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:adjustRightInd w:val=\"off\"/><w:spacing/><w:ind/><w:widowControl w:val=\"off\"/><w:suppressLineNumbers/><w:pBdr/><w:ind/></w:pPr><w:rPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:rFonts w:cs=\"Noto Sans Devanagari\"/></w:rPr></w:style>";

    char* header5="<w:style xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:type=\"character\" w:styleId=\"Hyperlink\"><w:name w:val=\"Hyperlink\"/><w:rsid w:val=\"006A55B0\"/><w:rPr><w:color w:val=\"000080\"/><w:u w:val=\"single\"/></w:rPr></w:style><w:style xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:type=\"character\" w:styleId=\"FollowedHyperlink\"><w:name w:val=\"FollowedHyperlink\"/><w:rsid w:val=\"006A55B0\"/><w:rPr><w:color w:val=\"800000\"/><w:u w:val=\"single\"/></w:rPr></w:style>";

    char* header6 = "<w:style xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:type=\"character\" w:styleId=\"CommentReference\"><w:name w:val=\"annotation reference\"/><w:basedOn w:val=\"DefaultParagraphFont\"/><w:semiHidden/><w:rsid w:val=\"007770B7\"/><w:rPr><w:sz w:val=\"16\"/><w:sz-cs w:val=\"16\"/></w:rPr></w:style><w:style xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:type=\"paragraph\" w:styleId=\"CommentText\"><w:name w:val=\"annotation text\"/><w:basedOn w:val=\"Normal\"/><w:semiHidden/><w:rsid w:val=\"007770B7\"/><w:pPr><w:pStyle w:val=\"CommentText\"/></w:pPr><w:rPr><w:sz w:val=\"20\"/><w:sz-cs w:val=\"20\"/></w:rPr></w:style><w:style xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:type=\"paragraph\" w:styleId=\"CommentSubject\"><w:name w:val=\"annotation subject\"/><w:basedOn w:val=\"CommentText\"/><w:next w:val=\"CommentText\"/><w:semiHidden/><w:rsid w:val=\"007770B7\"/><w:pPr><w:pStyle w:val=\"CommentSubject\"/></w:pPr><w:rPr><w:b/><w:b-cs/></w:rPr></w:style>\n    </w:styles>";

    char* header7="<w:docPr xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:displayBackgroundShape/><w:view w:val=\"print\"/><w:zoom w:percent=\"\"/><w:defaultTabStop w:val=\"709.2887\"/><w:docVars/></w:docPr>\n<w:body>";

    fwrite(header1, sizeof(char), strlen(header1), file);
    fwrite(header2, sizeof(char), strlen(header2), file);
    fwrite(header3, sizeof(char), strlen(header3), file);
    fwrite(header4, sizeof(char), strlen(header4), file);
    fwrite(header5, sizeof(char), strlen(header5), file);
    fwrite(header6, sizeof(char), strlen(header6), file);
    fwrite(header7, sizeof(char), strlen(header7), file);

    // Write some text to the file
    char* BeginingTxt="<w:p xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:pPr><w:pStyle w:val=\"Standard\"/></w:pPr><w:r><w:t>\n";

    char* EndinTxt="\n</w:t></w:r></w:p>";
    char sentence[255];

    printf("\n\tIngrese la oracion que desee y luego ENTER, escriba FIN y despues ENTER para finalizar:\n\n");
    while(strcmp(sentence,"FIN")!=0){
        printf("\t");
        scanf("%[^\n]%*c",sentence);
        if(strcmp(sentence,"FIN")!=0){

            fwrite(BeginingTxt, sizeof(char),strlen(BeginingTxt),file);
            fwrite(sentence, sizeof(char),strlen(sentence),file);
            fwrite(EndinTxt, sizeof(char),strlen(EndinTxt),file);
        }
    }
    Lines(15);

    //fwrite(text, sizeof(char), strlen(text), file);


    // escribir el footer del archivo xml
    char* footer="<w:p xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\"><w:pPr><w:pStyle w:val=\"Standard\"/></w:pPr></w:p><w:sectPr><w:type w:val=\"next-page\"/><w:pgSz xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:w=\"12241.5302\" w:h=\"15841.9803\" w:orient=\"portrait\"/><w:pgMar xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:top=\"1133.9978\" w:bottom=\"1133.9978\" w:left=\"1133.9978\" w:gutter=\"0\" w:right=\"1133.9978\"/><w:pgBorders xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\" w:offset-from=\"text\"/></w:sectPr></w:body></w:wordDocument>\n";
    fwrite(footer, sizeof(char), strlen(footer), file);

    // Close the file
    fclose(file);
}
void colocarPuntero(char* titleName,char* header1,char* header2,char* header3,char* header4,char* header5,char* header6,char* header7, char* BeginingTxt, char* EndinTxt){
    FILE *file;
    char* BufferBeginingTxt;
    char* BufferEndinTxt;
    char ContEnding[1];
    char EndinTxtCont[]="t></w:r></w:p>";
    char* Control;
    strcpy(Control,EndinTxtCont);
    file = fopen(titleName, "rb");
    int existBeginingTxt = 0;
    while(!feof(file)){

        fread(header1, sizeof(header1), 1, file);
        fread(header2, sizeof(header2), 1, file);
        fread(header3, sizeof(header3), 1, file);
        fread(header4, sizeof(header4), 1, file);
        fread(header5, sizeof(header5), 1, file);
        fread(header6, sizeof(header6), 1, file);
        fread(header7, sizeof(header7), 1, file);
        fread(BufferBeginingTxt, sizeof(BeginingTxt), 1, file);
        while(existBeginingTxt==0){////////////////////////////////////////////
            fread(ContEnding, 1, 1,file);
            if(strcmp(ContEnding,"<")==0){
                fread(ContEnding, 1, 1,file);
                if(strcmp(ContEnding,"/")==0){
                    fread(ContEnding, 1, 1,file);
                    if(strcmp(ContEnding,"w")==0){
                        fread(ContEnding, 1, 1,file);
                        if(strcmp(ContEnding,":")==0){
                            fread(EndinTxtCont, sizeof(EndinTxtCont), 1, file);

                        }
                    }
                }
            }
            if(strcmp(BufferBeginingTxt,BeginingTxt)==0){

            }/////////////////////
        }



    }
}
void elimArch(char* titleName) {
    int status;
    status = remove(titleName);
    Clean();
    Lines(13);
    if(status == 0) {
        printf("\n\tel archivo: %s \n\tfue eliminado exitosamente ;)\n", titleName);
    } else {
        printf("\n\tHijoles, no se pudo eliminar el archivo: %s\n", titleName);
        perror("\n\tError");
    }
}
