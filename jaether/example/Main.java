package example;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Random;

class Main {

    public static void test1(){
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

    public static void test2(){
        Random rnd = new Random();
        for(int i=0; i<10; i++){
            System.out.println(rnd.nextInt());
        }
    }

    public static void test3(){
        System.out.println(String.class.getName());
    }

    public static void test4(String str){
        if(str == null){
            System.out.println("NULL");
        } else System.out.println(str);
    }

    public static int test5(){
        int s = 0;
        for(int i=0; i<1000000; i++){
            Rectangle rekt = new Rectangle();
            rekt.nums[i&255] += i;
            s += rekt.nums[i&127];
        }
        return s;
    }

    public static void main(String[] args){
        System.out.println(test5());
    }
}