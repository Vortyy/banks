import std::io, std::ascii, std::io::file;
import std::collections::list;
import std::math::random;
import std::encoding::csv;

import raylib5::rl;

const usz MAX_CHAR = 10;

/* Expense && Account */
const String EMPTY = "No values";
const usz MAX_COST = 10;
const usz BUFSIZ = 8192;

String[4] auth = {"yohan", "lucile", "vero", "david"};

enum ExpenseType
{
  INCOME,
  OUTCOME
}

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

struct Expense
{
  int cost;
  ExpenseType type;
  String author;
  String description;
}

fn void Expense.init(&self, int cost, ExpenseType type, String author = EMPTY, String description = EMPTY){
  self.cost = cost;
  self.type = type;
  self.author = author;
  self.description = description;
}

fn String Expense.to_string(&self){
  return string::tformat("cost: %d, type: %s, aut: %s, desc: %s", self.cost, self.type, self.author, self.description);
}

fn ZString Expense.to_zstring(&self){
  return self.to_string().zstr_copy(allocator::heap());
}

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

fn int handle_error(fault err){
  io::printfn("Got an handled error: %s", err);
  return 1;
}

List{Expense}* expenses;
ZString[4] inputs;
int[4] current_pos = {0, 0, 0, 0};

fn void init_inputs(){
  for(int i = 0; i < inputs.len; i++){
    inputs[i] = (ZString) mem::new_array(char, MAX_CHAR + 1); 
    inputs[i][0] = '\0';
  }
}

fn void store_inputs(){
  io::printfn("%s", inputs);
  Expense exp;
  int price = inputs[0].str_view().to_int()!!;
  exp.init(price, INCOME, inputs[2].copy(allocator::heap()));
  expenses.push(exp);
}

fn void reset_inputs(){
  for (int i = 0; i < inputs.len; i++){
    inputs[i][0] = '\0';
    current_pos[i] = 0;
  }
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

  rl::initWindow(w, h, "raylib in fucking C3");
  rl::setTargetFPS(60);

  int[3] rectProp = {10, 150, 30};
  int selected = 0;

  init_inputs();

  while(!rl::windowShouldClose()){
    //UPDATE
    if(rl::isKeyPressed(rl::KEY_TAB)){
      selected++;
      if(selected > 3) selected = 0;
    }

    if(rl::isKeyPressed(rl::KEY_BACKSPACE)){
      current_pos[selected]--;
      if(current_pos[selected] < 0) current_pos[selected] = 0;
      int curr = current_pos[selected];
      inputs[selected][curr] = '\0';
    };

    if(rl::isKeyPressed(rl::KEY_ENTER)){
      store_inputs();
      reset_inputs();
    }
    
    int key = rl::getCharPressed();
    while(key > 0){
      if(current_pos[selected] < MAX_CHAR && ascii::is_alnum((char) key)){
        int curr = current_pos[selected];
        inputs[selected][curr] = (char) key;
        inputs[selected][curr + 1] = '\0';
        current_pos[selected]++;
      }
      key = rl::getCharPressed();
    }

    //DRAWING
    rl::@drawing(){
      rl::clearBackground(rl::RAYWHITE);
      
      // inputs 
      for(int i = 0; i < inputs.len; i++){
        int x = rectProp[0] + (i * 160);
        rl::drawRectangle(x, rectProp[0], rectProp[1], rectProp[2], rl::DARKGRAY);
        rl::drawText(inputs[i], x + 5, rectProp[0] + 5, 20, rl::RED);
        if(selected == i){
          rl::drawRectangleLines(x, rectProp[0], rectProp[1], rectProp[2], rl::RED);
        } else {
          rl::drawRectangleLines(x, rectProp[0], rectProp[1], rectProp[2], rl::BLACK);
        }
      }

      // expense list
      for(int i = 0; i < expenses.len(); i++){
        int y = 70 + 30 * i;
        rl::drawText(expenses.get(i).to_zstring(), 10, y, 20, rl::BLACK);
      }
    };

  }

  rl::closeWindow();
  return 0;
}
