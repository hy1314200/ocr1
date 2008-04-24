/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package manager;

/**
 *
 * @author liuyi
 */
public class Manager {
    public static boolean checkExist(char c){
        return false;
    }

    public static void checkExist(String str, StringBuffer exist, StringBuffer notExist) {
        int len = str.length();
        char[] array = str.toCharArray();
        for(int i = 0; i<len; i++){
            if(checkExist(array[i])){
                exist.append(array[i]);
            }else{
                notExist.append(array[i]);
            }
        }
    }
}
