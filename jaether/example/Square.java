package example;

public class Square extends Shape {
    public int b = 20;
    public static int B = 20;
    public Square(){
        super();
    }
    @Override
    public int area(){
        super.area();
        System.out.println("Square::area");
        return 3;
    }
}