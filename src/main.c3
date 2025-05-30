import std::io, std::io::file;
import std::ascii;
import std::time::datetime;
import std::collections::list;
import std::math::random;
import std::encoding::csv;

import raylib5::rl;

macro set_centered(box_size, elemnt_size){
  return (box_size / 2) - (elemnt_size / 2);
}

/* Expense && Account */
const String EMPTY = "No values";
const usz MAX_COST = 10;
const usz BUFSIZ = 8192;

faultdef
  UNKNOWN_EXPENSE_TYPE;

String[4] auth = {"yohan", "lucile", "vero", "david"};

/* Account */
struct Account
{
  String name;
  int total;
  List{Expense}* expenses;
}

fn void Account.init(&self, String name, List{Expense}* expenses)
{
  self.name = name;
  self.total = 0;
  self.expenses = expenses;
  // compute total
  self.getTotal();
}

fn void Account.getTotal(&self)
{
  for(int i = 0; i < self.expenses.len(); i++){
    Expense expense = self.expenses.get(i);
    self.total += expense.cost * (int) (expense.type == ExpenseType.INCOME)
                  - expense.cost * (int) (expense.type == ExpenseType.OUTCOME);
  }
}

enum ExpenseType
{
  INCOME,
  OUTCOME
}

/* Expense */
struct Expense
{
  int cost;
  ExpenseType type;
  DateTime date;
  String author;
  String description;
}

fn void Expense.init(&self, int cost, ExpenseType type, String author = EMPTY, String description = EMPTY){
  self.cost = cost;
  self.type = type;
  self.date = datetime::now();
  self.author = author;
  self.description = description;
}

fn String Expense.to_string(&self){
  return string::tformat("%s:%d:%s:%s", self.date.format(allocator::heap(), DATEONLY), self.cost, self.author, self.description);
}

fn ZString Expense.to_zstring(&self){
  return self.to_string().zstr_copy(allocator::heap());
}

fn ZString Expense.get_author(&self){
  return self.author.zstr_tcopy();
}

fn ZString Expense.get_description(&self){
  return self.description.zstr_tcopy();
}

fn ZString Expense.get_cost(&self){
  return string::tformat("%d", self.cost).zstr_tcopy();
}

fn ZString Expense.get_date(&self){
  return self.date.format(allocator::heap(), DATEONLY).zstr_tcopy();
}

/* Raylib input's */
const usz MAX_CHAR = 10;

struct Input 
{
  ZString content;
  int current_pos;
}

fn void Input.init(&self){
  self.content = (ZString) mem::new_array(char, MAX_CHAR + 1); 
  self.content[0] = '\0';
  self.current_pos = 0;
}

fn bool Input.is_empty(&self){
  return self.current_pos == 0;
}

fn void Input.reset(&self){
  self.content[0] = '\0';
  self.current_pos = 0;
}

fn void Input.add_char(&self, char key){
  if(self.current_pos < MAX_CHAR){
    self.content[self.current_pos] = key;
    self.content[self.current_pos + 1] = '\0';
    self.current_pos++;
  }
 }

fn void Input.remove_char(&self){
  self.current_pos--;
  if(self.current_pos < 0) self.current_pos = 0;
  self.content[self.current_pos] = '\0';
}

fn String Input.copy(&self){
  return self.content.copy(allocator::heap());
}

/* Utils */
fn void generate_exp(List{Expense}* expenses, usz nb){
  expenses.init(allocator::heap(), nb);

  SimpleRandom rand;
  Expense curr_exp;
  random::seed(&rand, 0);
  while(nb-- > 0){
    int val = random::next(&rand, MAX_COST);
    String author = auth[val%4];
    ExpenseType type = (val%2 == 0) ? INCOME : OUTCOME;  
    curr_exp.init(val, type, author);
    expenses.push(curr_exp);
  }
}

fn ExpenseType? getType(String content){
  if(content == "inc"){
    return INCOME;
  }

  return UNKNOWN_EXPENSE_TYPE?;
}

fn int handle_error(fault err){
  io::printfn("Got an handled error: %s", err);
  return 1;
}

int text_size = 10;
const usz DEFAULT_TEXT = 10;
const usz BIG_TEXT = 20;
List{Expense}* expenses;
Input[4] inputs;

fn void init_inputs(){
  for(int i = 0; i < inputs.len; i++){
    inputs[i].init();
  }
}

fn void reset_inputs(){
  for (int i = 0; i < inputs.len; i++){
    inputs[i].reset();
  }
}

/* store_inputs: try to store inputs if achieve it return true */
fn bool store_inputs(){
  // Validate data
  for(int i = 0; i < inputs.len; i++){
    if(inputs[i].is_empty()) return false;
  }
    
  int? value = inputs[0].copy().to_int();
  if(catch excuse = value) return false;

  ExpenseType? type = getType(inputs[1].content.str_view());
  if(catch excuse = type) return false;

  // create an expense and store it
  Expense exp;
  exp.init(value, type, inputs[2].copy(), inputs[3].copy());
  expenses.push(exp);

  return true;
}

fn int main(int argc, char** argv){
  File bank = file::open("./res/bank.csv", "r")!!;
  defer (void) bank.close();

  // char[] buffer = mem::new_array(char, BUFSIZ);
  // defer free(buffer);
  // usz r_bytes;
  // while((r_bytes = bank.read(buffer)!!) > 0){
  //   io::printfn("bytes read: %d", r_bytes);
  //   io::printfn("%s", (String) buffer[0:r_bytes]);
  // }

  CsvReader csv_r;
  CsvRow? row;
  csv_r.init(&bank);

  row = csv_r.read_row(allocator::heap());
  while(try unwrap_row = row){
    io::printfn("row len: %d, elmts: %s", unwrap_row.len(), unwrap_row.list);
    row = csv_r.read_row(allocator::heap());
  }

  if(catch err = row){
    if(err != io::EOF) return handle_error(err);
  }
  
  expenses = malloc(List{Expense}.sizeof);
  defer {
    expenses.free();
    free(expenses);
  }

  generate_exp(expenses, 4);
  io::printfn("Len -> %d, addr -> %p", expenses.len(), expenses); 

  for(int i = 0; i < expenses.len(); i++){
    io::printf("%d -> ", i);
    io::printn(expenses.get(i).to_string());
  }

  Account acc;
  acc.init("Yohan", expenses);
  io::printfn("total: %d", acc.total);

  int w = 800;
  int h = 450;
  int frm_counter = 0;

  rl::setConfigFlags(rl::FLAG_WINDOW_RESIZABLE);
  rl::initWindow(w, h, "raylib in fucking C3");
  rl::setTargetFPS(60);

  int[3] rectProp = {10, 150, 30};
  int selected = 0;

  init_inputs();

  while(!rl::windowShouldClose()){
    //UPDATE
    w = rl::getScreenWidth();
    h = rl::getScreenHeight();

    text_size = ( w <= 800 ) ? DEFAULT_TEXT : BIG_TEXT;
    

    if(rl::isKeyPressed(rl::KEY_TAB)){
      frm_counter = 0;
      selected++;
      if(selected > 3) selected = 0;
    }

    if(rl::isKeyPressed(rl::KEY_BACKSPACE)){
      inputs[selected].remove_char();
    };

    if(rl::isKeyPressed(rl::KEY_ENTER)){
      if(store_inputs()) reset_inputs();
    }
    
    int key = rl::getCharPressed();
    while(key > 0){
      char ckey = (char) key;
      if(ascii::is_alnum(ckey) || ascii::is_space(ckey)){
        inputs[selected].add_char(ckey); 
      }
      key = rl::getCharPressed();
    }

    frm_counter++;

    //DRAWING
    rl::@drawing(){
      rl::clearBackground(rl::RAYWHITE);
      
      // Inputs 
      for(int i = 0; i < inputs.len; i++){
        int x = rectProp[0] + (i * 160);
        rl::drawRectangle(x, rectProp[0], rectProp[1], rectProp[2], rl::DARKGRAY);
        rl::drawText(inputs[i].content, x + 5, rectProp[0] + 5, text_size, rl::RED);
        if(selected == i){
          rl::drawRectangleLines(x, rectProp[0], rectProp[1], rectProp[2], rl::RED);
          if(inputs[i].current_pos < MAX_CHAR){
            if((frm_counter/text_size) % 2 == 0) rl::drawText("_", x + 8 + rl::measureText(inputs[i].content, text_size) , rectProp[0] + 5, text_size, rl::RED);
          }
        } else {
          rl::drawRectangleLines(x, rectProp[0], rectProp[1], rectProp[2], rl::BLACK);
        }
      }

      // Draw expenses list container
      rl::drawRectangleLines(10, 60, w/2 - text_size, h - 100 , rl::BLACK);
     
      int box_size = ((w/2 - text_size) / 4);
      // Draw 3 lines + headers
      for(int i = 1; i < 4; i++){
        int x = box_size * i + 10;
        rl::drawLine(x, 60, x, h - 40, rl::BLACK);
      }

      // Expenses list
      for(int i = 0; i < expenses.len(); i++){
        Expense* ptr_exp = expenses.get_ref(i);
        int y = 70 + 30 * i;

        // Text measurement
        int author_size = rl::measureText(ptr_exp.get_author(), text_size);
        int description_size = rl::measureText(ptr_exp.get_description(), text_size);
        int cost_size = rl::measureText(ptr_exp.get_cost(), text_size);
        int date_size = rl::measureText(ptr_exp.get_date(), text_size);

        // Write text
        // TODO: check if box_size > measure text and write a substring ???
        rl::drawText(ptr_exp.get_date(), set_centered(box_size, date_size) + 10, y, text_size, rl::BLACK);//Date
        rl::drawText(ptr_exp.get_author(), set_centered(box_size, author_size) + (box_size + 10), y, text_size, rl::BLACK);//Author
        rl::drawText(ptr_exp.get_description(), set_centered(box_size, description_size) + (box_size * 2 + 10), y, text_size, rl::BLACK);//Description
        rl::drawText(ptr_exp.get_cost(), set_centered(box_size, cost_size) + (box_size * 3 + 10), y, text_size, rl::BLACK);//Cost
      }
    };
  }

  // TODO: File system to keep track of expenses
  // TODO: let part for keep monthly total
  // TODO: phone sized when screen width < x

  rl::closeWindow();
  return 0;
}
