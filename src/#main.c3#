import std::io, std::io::file;
import std::collections::list;
import std::math::random;
import std::encoding::csv;

import raylib5::rl;

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

fn int main(int argc, char** argv){
  io::printfn("RAYLIB_PI = %f", rl::PI);
  
  File bank = file::open("./res/bank.csv", "r")!!;
  defer (void) bank.close();

  char[] buffer = mem::new_array(char, BUFSIZ);
  defer free(buffer);

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
  
  List{Expense}* expenses = malloc(List{Expense}.sizeof);
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

  rl::Vector2 vec = {.x = 700, .y = 225};

  while(!rl::windowShouldClose()){
    //UPDATE
    

    //DRAWING
    rl::@drawing(){
      rl::clearBackground(rl::RAYWHITE);
      rl::drawRing(vec, 150.f, 170.f, 0.f, 360.f, 0, rl::BLACK);
      rl::drawText("Hello worlddoooooo !", 190, 200, 20, rl::LIGHTGRAY);
      rl::drawText("Yohan", 190, 220, 20, rl::RED);
    };
  }

  rl::closeWindow();
  
  return 0;
}
