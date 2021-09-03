package example;

import java.util.ArrayList;
import java.util.HashMap;

class Main {

    public static void main(String[] args){
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
            stuff.put(word, "haha");
        }
        for(String word : strings){
            sb.append(word);
        }
        System.out.println(666);
        System.out.println(sb.toString());

        for(String key : stuff.keySet()){
            System.out.println(key);
            System.out.println(stuff.get(key));
        }
    }
}