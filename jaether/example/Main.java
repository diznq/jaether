package example;

class Main {

    public static void main(String[] args){
        String[] arr = new String[]{"Hello, ", "world"};
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
            sb.append(word);
        }
        System.out.println(666);
        System.out.println(sb.toString());
    }
}