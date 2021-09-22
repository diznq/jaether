package example;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

import org.w3c.dom.CharacterData;

class Main {

    public static void test1() {
        String[] arr = new String[]{"Hello, ", "world!"};
        ArrayList<String> strings = new ArrayList<>();
        HashMap<String, String> stuff = new HashMap<>();
        Rectangle shape = new Rectangle();
        System.out.println(Rectangle.A);
        System.out.println(Rectangle.B);
        System.out.println(Rectangle.C);
        System.out.println(shape.a);
        System.out.println(shape.b);
        System.out.println(shape.c);
        System.out.println(shape.area());
        StringBuilder sb = new StringBuilder();
        for(String word : arr){
            System.out.println(word);
            strings.add(word);
            sb.append(word);
        }

        System.out.println(666);
        System.out.println(sb.toString());

        for(String word : strings){
            stuff.put(word, "haha");
        }

        for(String key : stuff.keySet()){
            System.out.println(key);
            System.out.println(stuff.get(key));
        }
    }

    public static void test2() {
        Random rnd = new Random();
        for(int i=0; i<10; i++) {
            System.out.println(rnd.nextInt());
        }
    }

    public static void test3() {
        System.out.println(String.class.getName());
        System.out.println(int.class.getName());
    }

    public static void test4(String str) {
        if(str == null){
            System.out.println("NULL");
        } else System.out.println(str);
    }

    public static int test5() {
        int s = 0;
        for(int i=0; i<1000000; i++){
            Rectangle rekt = new Rectangle();
            rekt.nums[i&255] += i;
            s += rekt.nums[i&127];
        }
        return s;
    }

    public static void test6(){
        try {
            throw new Exception("msg");
        } catch(Exception ex){
            System.out.println(ex.getMessage());
        }
    }

    public static void test7(int a, int b, int c){
        System.out.println(a);
        System.out.println(b);
        System.out.println(c);
    }

    public static void test8(long a, long b, long c){
        System.out.println(a);
        System.out.println(b);
        System.out.println(c);
    }

    public static void main(String[] args) { 
        test2();
    }
}