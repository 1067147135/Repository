from turtle import Turtle, Screen
import random

key_up, key_down, key_left, key_right, key_space = "Up", "Down", "Left", "Right", "space"
g_snake = None  # define a initial value for object snake
g_monster = None    # define a initial value for object monster
g_screen = None # define a initial value for object screen
g_text = None   # used to print welcome words
g_flag = True   # used to determine whether the snake should move now
g_foods = []    # used to store the data of foods
g_start_flag = True # used to make the start method only work once
g_snake_lenth = 5   # used to store the lenth of the snake should be
g_snake_extend = 1  # used to store the lenth of the snake right now
g_monster_flag = True   # used to determine whether the monster should move now
g_cont = 0  # store the contacted times
g_time = 0  # store the games
g_body = [] # store the position data of snake's body
g_title_flag = True # determine whether to update the title

def start(x,y): # click, and start the game
    global g_start_flag
    if g_start_flag:
        g_start_flag = False
        title()
        g_text.clear()
        configure_food()
        configure_key(g_screen)
        chase()
        move()
        g_screen.update()

def welcome_words():    # print the welcome words
    w = Turtle()
    w.up()
    w.hideturtle()
    w.goto(-200,205)
    w.write("Welcome to Snake!", align="left", font=("Arial", 12, "normal"))
    w.goto(-200,175)
    w.write("You are going to use 4 arrow keys to move the snake", align="left", font=("Arial", 12, "normal"))
    w.goto(-200,155)
    w.write("around the screen, trying to consume all the food items", align="left", font=("Arial", 12, "normal"))
    w.goto(-200,135)
    w.write("before the monster catches you!", align="left", font=("Arial", 12, "normal"))
    w.goto(-200,105)
    w.write("Click anywhere on the screen to start the game, have fun!", align="left", font=("Arial", 12, "normal"))
    return w

def configure_screen(): # configure the screen
    s = Screen()
    s.setup(500,500)
    s.title('Snake: Contacted:0, Time:0')
    s.tracer(0)
    return s

def title():    # update the title
    global g_title_flag
    if g_title_flag:
        global g_cont
        global g_time
        for (x1,y1) in g_body:
            if g_monster.distance(x1,y1) <= 20:
                g_cont += 1
        g_time += 1
        tit = 'Snake: Contacted:' + str(g_cont) + ', Time:' + str(g_time)
        g_screen.title(tit)
        g_screen.update()
        g_screen.ontimer(title,1000)

def configure_snake(shape="square", color="red", x=0, y=0): # configure the snake
    t = Turtle(shape)
    t.up()
    t.color(color)
    t.goto(x,y)
    return t

def configure_monster(shape="square", color="purple"):   # configure the monster
    m = Turtle(shape)
    m.up()
    m.color(color)
    x = 0
    y = 0
    while -150<x<150:   # place the monster far enough
        x = random.randint(-200,200)
        y = random.randint(-200,-150)
    m.goto(x,y)
    return m

def configure_food():   # configure the food
    f = Turtle()
    f.up()
    f.hideturtle()
    number = 1
    while len(g_foods) < 9:
        x = random.randint(-240,240)
        y = random.randint(-240,240)
        if x == 0 and y == 0:
            continue
        elif (x,y) in g_foods:
            continue
        else:
            g_foods.append((x,y,number))
            number += 1
    for i in range(9):
        x,y,z = g_foods[i]
        f.goto(x,y)
        f.write(str(z), align="left", font=("Arial", 12, "normal"))
    return f
    
def move_up():  # change the direction
    g_snake.setheading(90)
    g_screen.update()

def move_down():    # change the direction
    g_snake.setheading(270)
    g_screen.update()

def move_left():    # change the direction
    g_snake.setheading(180)
    g_screen.update()

def move_right():   # change the direction
    g_snake.setheading(0)
    g_screen.update()

def flag_change():  # change the flag to determine whether the snake should move
    global g_flag
    g_flag = not g_flag

def check_move():   # check whether the snake touch the boundary at previous direction
    x,y = g_snake.xcor(),g_snake.ycor()
    if g_snake.heading()==90 and y>=240:
        return False
    elif g_snake.heading()==270 and y<=-240:
        return False
    elif g_snake.heading()==180 and x<=-240:
        return False
    elif g_snake.heading()==0 and x>=240:
        return False
    if g_flag == False: # check whether the snake should move
        return False
    return True

def move(): # move the snake
    if check_move():
        copy(g_snake.heading(),250)
        g_snake.clearstamps(1)
        del g_body[0]   # delete useless data to update the position data
        eat()
    extend()
    check_win()
    g_screen.update()
    g_screen.ontimer(move,10)

def configure_key(s):   # match the keyboard and the function
    s.onkey(move_up,key_up)
    s.onkey(move_down,key_down)
    s.onkey(move_left,key_left)
    s.onkey(move_right,key_right)
    s.onkey(flag_change,key_space)

def chase():    # move the monster
    global g_flag
    global g_monster_flag
    global g_title_flag
    if g_monster_flag:
        game_over = Turtle()
        game_over.up()
        game_over.hideturtle()
        x1,y1 = g_snake.xcor(),g_snake.ycor()
        x2,y2 = g_monster.xcor(),g_monster.ycor()
        distance = g_monster.distance(x1,y1)
        if distance >= 23:  # if the snake and the monster are far enough
            if abs(x1-x2) < abs(y1-y2):
                if y1-y2 > 0:
                    g_monster.setheading(90)
                else:
                    g_monster.setheading(270)
            else:
                if x1-x2 < 0:
                    g_monster.setheading(180)
                else:
                    g_monster.setheading(0)        
            speed = random.randint(15,25)   # the monster should have a random speed around snake's speed
            g_monster.forward(speed)
            g_screen.update()
            g_screen.ontimer(chase,500)
        else: # if the snake and the monster are close enough, game over
            g_flag = False
            g_title_flag = False
            game_over.goto(x1,y1)
            game_over.pencolor("red")
            game_over.write("Game over!!", align="right", font=("Arial", 12, "bold"))
                   
def copy(heading=0,update=500): # make stamps
    while True:
        if check_move():
            x1,y1 = g_snake.xcor(),g_snake.ycor()
            g_body.append((x1,y1))  # store the postion data
            g_snake.color('black','blue')
            g_snake.stamp()
            g_snake.setheading(heading)
            g_screen.ontimer(g_snake.forward(20), update)
            g_snake.color('red')
            g_screen.update()
            break
        else:    
            g_screen.update()
            continue
            
def extend():   # extend the length
    global g_snake_extend
    global g_snake_lenth
    while g_snake_extend < g_snake_lenth:
        if check_move():
            copy(g_snake.heading())
            g_snake_extend += 1
            eat()
        g_screen.ontimer(None,200)
        g_screen.update()

def eat():  # check eating
    global g_snake_lenth
    e = Turtle()
    e.up()
    e.hideturtle()
    for (x,y,z) in g_foods:
        distance = g_snake.distance(x+5,y+10)
        if distance <= 15:
            e.goto(x,y)
            e.pencolor("white")
            e.write(str(z), align="left", font=("Arial", 12, "normal"))
            g_foods.remove((x,y,z))
            g_snake_lenth += z
            g_screen.update()
            
def check_win():    # check whether all the foods are consumed
    global g_monster_flag
    global g_flag
    global g_title_flag
    e = Turtle()
    e.up()
    e.hideturtle()
    if len(g_foods) == 0:
        g_flag = False
        g_title_flag =False
        g_monster_flag = False
        x,y = g_snake.xcor(),g_snake.ycor()
        e.goto(x,y)
        e.pencolor("red")
        e.write("Winner!!", align="right", font=("Arial", 12, "bold"))
        g_screen.update()

if __name__ == "__main__":  # main function
    g_screen = configure_screen()
    g_snake = configure_snake()
    g_monster = configure_monster()    
    g_text = welcome_words()
    g_screen.update()
    g_screen.onclick(start)
    g_screen.listen()
    g_screen.mainloop()

