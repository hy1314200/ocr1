/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package manager;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Arrays;

/**
 *
 * @author liuyi
 */
public class Manager {
    
    private static final String currLibPath = "data/font/library";
    
    //private final String finalLibPath = "data/font/finalLib";
    
    private static final String tempDirPath = "tmp";
    
    private static final String appendFileName = "append";
    
    private static final String notValidFileName = "notValid";
    
    private static char[] library = null;

    public static boolean checkExist(char c) throws Exception {
        if(library == null){
            loadCurrLib();
        }
        
        int index = Arrays.binarySearch(library, c);        
        return index >= 0;
    }

    private static void appendCharsHelp(StringBuffer notValid) throws Exception {
        BufferedReader reader = null;
        try {
            checkDir(tempDirPath);
            String cmd = "ocr -a " + tempDirPath + "/" + appendFileName + " > " + tempDirPath + "/" + notValidFileName;
            Process child = Runtime.getRuntime().exec(cmd);
            child.waitFor();
            copyAppendFileAndUpdate();
            reader = new BufferedReader(new InputStreamReader(new FileInputStream(tempDirPath + "/" + notValidFileName)));
            notValid.append(reader.readLine());

            reader.close();
        } catch (InterruptedException ex) {
            throw new Exception("程序运行中断，添加失败");
        } catch (FileNotFoundException ex) {
            throw new Exception("找不到文件 " + tempDirPath + "/" + appendFileName + "，添加失败");
        } catch (IOException ex) {
            throw new Exception("输入输出流出错，添加失败");
        }
    }

    private static void checkDir(String tempDirPath) {
        File dir = new File(tempDirPath);
        if(!dir.exists()){
            dir.mkdirs();
        }
    }

    private static void copyAppendFileAndUpdate() throws Exception {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(new FileInputStream(tempDirPath + "/" + appendFileName)));
            FileOutputStream fos = new FileOutputStream(currLibPath);
            fos.write(reader.readLine().getBytes());

            reader.close();
            fos.close();

            loadCurrLib();
        } catch (FileNotFoundException ex) {
            throw new Exception("找不到文件 " + tempDirPath + "/" + appendFileName + "，添加失败");            
        }catch (IOException ex) {
            throw new Exception("输入输出流出错，添加失败");
        } 
    }

    private static void loadCurrLib() throws Exception {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(new FileInputStream(currLibPath)));
            library = reader.readLine().toCharArray();

            reader.close();
        } catch (FileNotFoundException ex) {
            throw new Exception("找不到文件 " + currLibPath);
        } catch (IOException ex) {
            throw new Exception("输入输出流出错");
        } 
    }
    
    private static String trimAndOrder(String str){
        str = str.replaceAll("\\s", "");
        
        char[] array = str.toCharArray();
        java.util.Arrays.sort(array);
        
        StringBuffer sb = new StringBuffer();
        sb.append(array[0]);
        int offset = 0, len = array.length;
        for(int i = 1; i<len; i++){
            if(array[i] != sb.charAt(offset)){
                sb.append(array[i]);
                ++offset;
            }
        }
        
        return sb.toString();
    }
    
    private static void checkExistWithoutOrder(String str, StringBuffer exist, StringBuffer notExist) throws Exception {        
        int len = str.length();
        char[] array = str.toCharArray();
        for (int i = 0; i < len; i++) {
            if (checkExist(array[i])) {
                if (exist != null) {
                    exist.append(array[i]);
                }
            } else {
                if (notExist != null) {
                    notExist.append(array[i]);
                }
            }
        }
    }

    public static void checkExist(String str, StringBuffer exist, StringBuffer notExist) throws Exception{
        str = trimAndOrder(str);
        checkExistWithoutOrder(str, exist, notExist);
    }
    
    public static void appendChars(String str, StringBuffer exist, StringBuffer notValid) throws Exception {

        FileOutputStream fos = null;
        try {
            str = trimAndOrder(str);
            checkExistWithoutOrder(str, exist, null);
            checkDir(tempDirPath);
            File file = new File(tempDirPath, appendFileName);
            if (!file.exists()) {
                file.createNewFile();
            }
            fos = new FileOutputStream(file);
            fos.write(str.getBytes());
            fos.close();

            appendCharsHelp(notValid);
        } catch (IOException ex) {
            throw new Exception("输入输出流出错");
        } 
    }    
}
