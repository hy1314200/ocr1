/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package manager;

import charlibmanager.CharLibManagerView;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.Arrays;
import javax.swing.JTextArea;

/**
 *
 * @author liuyi
 */
public class Manager {
    
    private static final String currLibPath = "data/font/curr/library";
    
    private static final String tempDirPath = "tmp";
    
    private static final String appendFileName = "append";
    
    private static final String trainedFileName = "needTrain";
    
    private static String library = new String();
    
    private static CharLibManagerView view;

    public static void boundView(CharLibManagerView tview) {
        view = tview;
    }

    public static boolean checkExist(char c) throws Exception {
        int index = Arrays.binarySearch(library.toCharArray(), c);
        return index >= 0;
    }
    
    public static boolean needTrain(){
        try {
            BufferedReader reader = new BufferedReader(new FileReader(trainedFileName)); 
            
            if(Boolean.valueOf(reader.readLine()) == true){
                reader.close();
                
                return true;
            }
        } catch(IOException ex) {
            enableTrain(false);
        }
        
        return false;
    }

    public static void enableTrain(boolean enable) {
        File file = new File(trainedFileName);
        
        try {
            if(!file.exists()){
                file.createNewFile();
            }
            
            PrintWriter writer = new PrintWriter(file);
            if(enable){
                writer.print("true");
            }else{
                writer.print("false");
            }
            writer.close();
        } catch(IOException ex) {
            ex.printStackTrace();
        }
    }

    public static String getLibrary() {
        return library;
    }

    public static void recogniseImage(String text, JTextArea displayArea) {
        String cmd = "E:/Project/VS2005/ocr/ocr/ocr.exe -r " + text;
        
        new Thread(new CmdThread(cmd, displayArea)).start();
    }

    public static void trainClassifier(JTextArea displayArea) {
        new Thread(new CmdThread("E:/Project/VS2005/ocr/ocr/ocr.exe -train", displayArea)).start();
    }

    private static void appendCharsHelp(StringBuffer exist, StringBuffer notValid) throws Exception {
        try {
            checkDir(tempDirPath);
            String cmd = "E:/Project/VS2005/ocr/ocr/ocr.exe -a " + tempDirPath + "/" + appendFileName;
            Process process = Runtime.getRuntime().exec(cmd);
            
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String buffer;
            while ((buffer = reader.readLine()) != null) {                
                if(buffer.indexOf("exist") != -1){
                    exist.append(buffer.substring(buffer.indexOf("=") + 1));                
                }else if(buffer.indexOf("not valid") != -1){            
                    notValid.append(buffer.substring(buffer.indexOf("=") + 1));
                }else{
                    throw new Exception("ocr.exe 执行出错");
                }
            }
            reader.close();
            
            int exitCode = process.waitFor();
            if (exitCode != 0) {
                throw new Exception("ocr.exe 执行出错");
            }

            loadCurrLib();
        } catch (InterruptedException ex) {
            throw new Exception("ocr.exe 运行中断，添加失败");
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

    public static void loadCurrLib() throws Exception {
        BufferedReader reader = null;
        try {
            reader = new BufferedReader(new InputStreamReader(new FileInputStream(currLibPath)));
            library = reader.readLine();
            Arrays.sort(library.toCharArray());

            reader.close();
        } catch (FileNotFoundException ex) {            
            // currunt library size is 0
        } catch (IOException ex) {
            throw new Exception("输入输出流出错");
        } 
        
        if(view != null){
            view.updateCharNum(library.length());
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

    public static void checkExist(String str, StringBuffer exist, StringBuffer notExist) throws Exception{
        str = trimAndOrder(str);
        
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
    
    public static void appendChars(String str, StringBuffer exist, StringBuffer notValid) throws Exception {
        PrintWriter writer = null;
        try {
            checkDir(tempDirPath);
            File file = new File(tempDirPath, appendFileName);
            if (!file.exists()) {
                file.createNewFile();
            }
            writer = new PrintWriter(file);
            
            writer.print(str);
            writer.close();

            appendCharsHelp(exist, notValid);
        } catch (IOException ex) {
            throw new Exception("输入输出流出错");
        } 
    }    
}
