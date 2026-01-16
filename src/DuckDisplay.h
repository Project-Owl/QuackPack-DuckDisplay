class DuckDisplay {
protected:
    size_t width, height;
public:
    DuckDisplay();
    void showDefaultScreen();
    size_t getWidth() {return width;}
    size_t getHeight() {return height;}

}
