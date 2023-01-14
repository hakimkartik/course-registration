#include <iostream>

using namespace std;

#include <string> 
#include <iostream> 

using namespace std;

const int COUNTER_THRESHOLD = 150 ;

class EncDec
{
public:
  int encrypt_digit(int tmp){
    // 48-57 : ACII range for string digits
    int offset = 4;
    int counter = 0;

    if (tmp >= 48 && tmp <= 57){

      while (1){
        if (tmp >= 48 && tmp <= 57) {
          tmp += 1 ;
          counter += 1 ;
        }

        if (tmp > 57){
          tmp = 48;
        }
        if (counter >= offset || counter > COUNTER_THRESHOLD){break;}
      }
    }
    return tmp;
  }

int encrypt_char(int tmp){
    // 65-90 : ACII range for string A-Z
    // 97-122 :  ACII range for string a-z

    int offset = 4;
    int counter = 0;
    
    while (counter < offset)
    {
        if ( (tmp >= 65 && tmp <= 90) || (tmp >= 97 && tmp <= 122) )
        {
            tmp++;
            counter++ ;
        }
        
        if (tmp > 90 && tmp < 97){
            tmp = 65;
        }
        else if (tmp > 122){
            tmp = 97;
        }
        
    }
    return tmp;
  }

string encrypt(string s1) {

    int N = s1.length();
    string result;
    if (N < 1){
      cout << "No valid string passed" << '\n';
      result = "";
    }
    else {

    for (int i = 0; i < N; i++) 
    {

    // 48-57 :  ACII range for string digits
    // 65-90 :  ACII range for string A-Z
    // 97-122 :  ACII range for string a-z
      if (isalpha(s1[i]))
      {
         
        char c = s1[i];
        int tmp = (int) c ;
        tmp = encrypt_char(tmp);
        s1[i] = (char) tmp;
        if (isupper(c))
        {
          s1[i] = toupper(s1[i]);
        }
        if (islower(c))
        {
          s1[i] = tolower(s1[i]);
        }
      }

      if (isdigit(s1[i]))
      {
          int tmp = encrypt_digit((int)s1[i]);
          s1[i] = (char) tmp ;
      }

    }
    result = s1;
    }
    return result;
  }

  int decrypt_digit(int tmp)
  {
    int offset = 4;
    int counter = 0;

    if (tmp >= 48 && tmp <= 57 ){

      while (1) {
          if (tmp >= 48 && tmp <= 57 ){
            tmp -= 1;
            counter += 1;
          }
          else{
            tmp = 57;
          }

        if (counter == offset || counter > COUNTER_THRESHOLD){break;}
      }
    }
    else{
      cout<< "Cannot decrypt digit" << '\n';
    }
    return tmp;

  }

  int decrypt_lowercase_char(int tmp){
    // 97-122 :  ACII range for string a-z
    int offset = 4;
    int counter = 0;

    if (tmp >= 97 && tmp <= 122 ){

      while (1) {
          if (tmp >= 97 && tmp <= 122 ){
            tmp -= 1;
            counter += 1;
          }
          else{
            tmp = 122;
          }

        if (counter == offset || counter > COUNTER_THRESHOLD){
          break;
        }
      }
    }
    else{
      cout << "Cannot decrypt lowercase char " << '\n';
    }
    return tmp;

  }

  int decrypt_uppercase_char(int tmp){
    // 65-90 :  ACII range for string A-Z
    int offset = 4;
    int counter = 0;

    if (tmp >= 65 && tmp <= 90 ){

      while (1) {
          if (tmp >= 65 && tmp <= 90 ){
            tmp -= 1;
            counter += 1;
          }
          else{
            tmp = 90;
          }

        if (counter == offset || counter > COUNTER_THRESHOLD){
          break;
        }

      }
    }
    else{
      cout << "Cannot decrypt uppercase char " << '\n';
    }
    return tmp;

  }


  string decrypt(string s1){
    // 48-57 :  ACII range for string digits
    // 65-90 :  ACII range for string A-Z
    // 97-122 :  ACII range for string a-z
    int N = s1.length();
    string result;
    if (N < 1){
      cout << "No valid string passed" << '\n';
      result = "";
    }
    else{
      int offset = 4;

      for (int i = 0; i < N; i++) {
        int counter = 1;

        char c = s1[i];
        int tmp = (int) c;
        if (tmp >=48 && tmp <= 57)
        {
          tmp = decrypt_digit(tmp);
        }
        else if (tmp >=65 && tmp <= 90)
        {
          tmp = decrypt_uppercase_char(tmp);
        }
        else if (tmp >=97 && tmp <= 122)
        {
          tmp = decrypt_lowercase_char(tmp);
        }
        s1[i] = (char) tmp;
      }
      result = s1;
    }

    return result;
  }


};