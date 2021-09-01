package example;

public class Square extends Shape {
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