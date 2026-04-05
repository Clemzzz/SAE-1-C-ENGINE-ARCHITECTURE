#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <time.h> // Nécessaire pour l'aléatoire

#include "board.h"

/* Maximum de caractères pour les noms des joueurs */
#define NAME_MAX 64

/* Codes couleur (ANSI) pour améliorer l'affichage. */
#define RESET "\x1b[0m"
#define DIM "\x1b[2m"
#define GREEN "\x1b[32m"
#define CYAN "\x1b[36m"
#define MAGENTA "\x1b[35m"
#define YELLOW "\x1b[33m" // Ajout pour le bot
#define RED "\x1b[31m"    // Ajout pour les erreurs critiques

#define UPPER_LEFT_CORNER "\u250C"
#define UPPER_RIGHT_CORNER "\u2510"
#define LOWER_LEFT_CORNER "\u2514"
#define LOWER_RIGHT_CORNER "\u2518"
#define HORIZONTAL_LINE "\u2500"
#define VERTICAL_LINE "\u2502"

void clear_screen(void)
{
  printf("\033[H\033[J");
}

const char *piece_color(size piece)
{
  switch (piece)
  {
  case ONE:
    return GREEN;
  case TWO:
    return CYAN;
  case THREE:
    return MAGENTA;
  default:
    return RESET;
  }
}

int player_index(player p)
{
  switch (p)
  {
  case SOUTH_P:
    return 0;
  case NORTH_P:
    return 1;
  default:
    return -1;
  }
}

const char *player_side_name(player p)
{
  switch (p)
  {
  case SOUTH_P:
    return "Sud";
  case NORTH_P:
    return "Nord";
  default:
    return "Aucun";
  }
}

const char *player_display_name(const char names[NB_PLAYERS][NAME_MAX], player p)
{
  int idx = player_index(p);
  if (idx < 0)
  {
    return "???";
  }
  return names[idx];
}

void discard_line(void)
{
  int ch = 0;
  while ((ch = getchar()) != '\n' && ch != EOF)
  {
  }
}

bool read_int_min_max(const char *prompt, int min, int max, int *value)
{
  while (1)
  {
    printf("%s", prompt);
    int input = 0;
    int read = scanf("%d", &input);
    if (read == EOF)
    {
      return false;
    }
    if (read != 1)
    {
      printf("Erreur : nombre entier attendu.\n");
      discard_line();
      continue;
    }
    if (input < min || input > max)
    {
      printf("Veuillez entrer une valeur entière entre %d et %d.\n", min, max);
      discard_line();
      continue;
    }
    discard_line();
    if (value)
    {
      *value = input;
    }
    return true;
  }
}

bool read_board_index(const char *prompt, int *index)
{
  int raw = 0;
  if (!read_int_min_max(prompt, 1, DIMENSION, &raw))
  {
    return false;
  }
  if (index)
  {
    *index = raw - 1;
  }
  return true;
}

bool ask_name(char *dest, size_t size, const char *side_label)
{
  (void)size;
  while (1)
  {
    printf("Nom du joueur %s : ", side_label);
    if (scanf("%63s", dest) != 1)
    {
      return false;
    }
    discard_line();
    return true;
  }
}

bool has_remaining_pieces(board game, player p)
{
  for (size piece = ONE; piece <= THREE; ++piece)
  {
    if (nb_pieces_available(game, piece, p) > 0)
    {
      return true;
    }
  }
  return false;
}

void print_board(board game, const char names[NB_PLAYERS][NAME_MAX], player current)
{
  clear_screen();
  const char *north_name = player_display_name(names, NORTH_P);
  const char *south_name = player_display_name(names, SOUTH_P);

  for (int col = 0; col < DIMENSION; ++col)
  {
    printf("   %s%d%s", DIM, col + 1, RESET);
  }
  printf("\n        %s", UPPER_LEFT_CORNER);
  for (size_t i = 0; i < 9; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s", UPPER_RIGHT_CORNER);
  printf("\n        %s    N    %s", VERTICAL_LINE, VERTICAL_LINE);
  printf("\n %s", UPPER_LEFT_CORNER);
  for (size_t i = 0; i < 6; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s---------%s", LOWER_RIGHT_CORNER, LOWER_LEFT_CORNER);
  for (size_t i = 0; i < 6; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s\n", UPPER_RIGHT_CORNER);

  for (int line = DIMENSION - 1; line >= 0; --line)
  {
    printf(" %s", VERTICAL_LINE);
    for (int col = 0; col < DIMENSION; ++col)
    {
      if (col != 0)
        printf(" ");
      size current = get_piece_size(game, line, col);
      if (current == NONE)
        printf(" %s.%s ", DIM, RESET);
      else
        printf(" %s%c%s ", piece_color(current), (char)('0' + current), RESET);
    }
    printf("%s  %s%d%s\n", VERTICAL_LINE, DIM, line + 1, RESET);
    if (line != 0)
      printf(" %s                       %s\n", VERTICAL_LINE, VERTICAL_LINE);
  }
  printf(" %s", LOWER_LEFT_CORNER);
  for (size_t i = 0; i < 6; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s---------%s", UPPER_RIGHT_CORNER, UPPER_LEFT_CORNER);
  for (size_t i = 0; i < 6; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s\n", LOWER_RIGHT_CORNER);
  printf("        %s    S    %s\n", VERTICAL_LINE, VERTICAL_LINE);
  printf("        %s", LOWER_LEFT_CORNER);
  for (size_t i = 0; i < 9; i++)
    printf("%s", HORIZONTAL_LINE);
  printf("%s\n", LOWER_RIGHT_CORNER);
  printf("     ");
  (current == NORTH_P) ? printf("%s> ", GREEN) : printf("  ");
  printf("Nord (ligne %d) : %s%s\n", DIMENSION, north_name, RESET);
  printf("     ");
  (current == SOUTH_P) ? printf("%s> ", GREEN) : printf("  ");
  printf("Sud (ligne %d) : %s%s\n\n", 1, south_name, RESET);
}

void print_remaining_pieces(board game, player p)
{
  printf("Pièces restantes :\n");
  for (size piece = ONE; piece <= THREE; ++piece)
  {
    int left = nb_pieces_available(game, piece, p);
    if (left < 0)
      left = 0;
    printf("  taille %d : %d pièce(s) restante(s)\n", piece, left);
  }
}

const char *return_code_message(return_code rc)
{
  switch (rc)
  {
  case OK:
    return "opération réussie";
  case EMPTY:
    return "la case est vide";
  case FORBIDDEN:
    return "coup interdit";
  case PARAM:
    return "paramètre invalide";
  default:
    return "code inconnu";
  }
}

void display_ascii_art(const char *filename)
{
  FILE *f = fopen(filename, "r");

  if (f == NULL)
  {
    printf("Erreur : Impossible d'ouvrir %s\n", filename);
    return;
  }

  char buffer[256]; // Une ligne peut faire max 255 caractères

  while (fgets(buffer, sizeof(buffer), f) != NULL)
  {
    printf("%s", buffer);
  }

  fclose(f);
}

/* ========================================================= Logique BOT ========================================================= */

/**
 * Tour du bot lors du setup.
 */
void bot_setup_turn(board game, int *current_counts, char names[NB_PLAYERS][NAME_MAX], player current)
{
  printf("%sLe Bot (%s) réfléchit au placement...%s\n", YELLOW, names[1], RESET);

  int size_choice = 0;
  int column_choice = 0;
  bool found = false;

  // Essayer aléatoirement jusqu'à trouver un coup valide (max 200 tentatives)
  int attempts = 0;
  while (!found && attempts < 200)
  {
    int s_idx = rand() % 3;   // 0, 1, 2
    int c = (rand() % 6) + 1; // 1-6

    if (current_counts[s_idx] > 0)
    {
      // On teste sur une copie
      board temp = copy_game(game);
      if (place_piece(temp, (size)(s_idx + 1), current, c) == OK)
      {
        size_choice = s_idx + 1;
        column_choice = c;
        found = true;
      }
      destroy_game(temp);
    }
    attempts++;
  }

  // Placement réel
  place_piece(game, (size)size_choice, current, column_choice);
  current_counts[size_choice - 1]--;
  printf("Le Bot a placé une pièce de taille %d en colonne %d.\n", size_choice, column_choice);
}

/**
 * Tour du bot lors de la phase principale.
 */
void bot_main_turn(board game, char names[NB_PLAYERS][NAME_MAX], player current)
{
  printf("%sLe Bot (%s) joue...%s\n", YELLOW, names[1], RESET);

  // Trouve toutes les pièces que le bot peut bouger
  // On stocke les coordonnées (line, col) valides
  typedef struct
  {
    int l;
    int c;
  } Pos;
  Pos candidates[DIMENSION * DIMENSION];
  int count = 0;

  for (int l = 0; l < DIMENSION; l++)
  {
    for (int c = 0; c < DIMENSION; c++)
    {
      // Simulation de pick
      board temp = copy_game(game);
      if (pick_piece(temp, current, l, c + 1) == OK)
      {
        // Vérifier s'il y a au moins un mouvement possible
        bool can_move_anywhere = false;
        const direction dirs[] = {NORTH, SOUTH, EAST, WEST, GOAL};
        for (int i = 0; i < 5; i++)
        {
          if (is_move_possible(temp, dirs[i]))
          {
            can_move_anywhere = true;
            break;
          }
        }
        if (can_move_anywhere)
        {
          candidates[count].l = l;
          candidates[count].c = c + 1;
          count++;
        }
      }
      destroy_game(temp);
    }
  }

  if (count == 0)
  {
    printf("Le bot ne peut bouger aucune pièce !\n");
    return;
  }

  // Choisit une pièce au hasard
  int r = rand() % count;
  pick_piece(game, current, candidates[r].l, candidates[r].c);
  printf("Le bot sélectionne la pièce en (%d, %d).\n", candidates[r].l, candidates[r].c);
  print_board(game, names, current);

  // Déplace la pièce
  while (picked_piece_owner(game) != NO_PLAYER)
  {
    const direction dirs[] = {NORTH, SOUTH, EAST, WEST, GOAL};
    direction valid_dirs[5];
    int valid_count = 0;
    bool winning_move = false;

    for (int i = 0; i < 5; i++)
    {
      if (is_move_possible(game, dirs[i]))
      {
        if (dirs[i] == GOAL)
          winning_move = true;
        valid_dirs[valid_count++] = dirs[i];
      }
    }

    direction choice;
    if (winning_move && is_move_possible(game, GOAL))
    {
      choice = GOAL;
    }
    else if (valid_count > 0)
    {
      choice = valid_dirs[rand() % valid_count];
    }
    else
    {
      cancel_movement(game);
      return;
    }

    move_piece(game, choice);

    print_board(game, names, current);
  }
}

bool setup_phase(board game, char names[NB_PLAYERS][NAME_MAX], bool is_pve)
{
  player current = SOUTH_P;
  int nb_sud[] = {2, 2, 2};
  int nb_nord[] = {2, 2, 2};
  int *current_counts;

  printf("\n%s========================================== Phase de mise en place ==========================================%s\n", GREEN, RESET);

  while (has_remaining_pieces(game, SOUTH_P) || has_remaining_pieces(game, NORTH_P))
  {
    if (!has_remaining_pieces(game, current))
    {
      current = next_player(current);
      continue;
    }

    if (current == SOUTH_P)
      current_counts = nb_sud;
    else
      current_counts = nb_nord;

    print_board(game, names, current);

    // SI C'EST LE TOUR DU BOT
    if (is_pve && current == NORTH_P)
    {
      bot_setup_turn(game, current_counts, names, current);
      current = next_player(current);
      continue;
    }

    // SI C'EST L'HUMAIN QUI JOUE
    int idx = player_index(current);
    printf("Placement pour %s (%s)\n", names[idx], player_side_name(current));
    print_remaining_pieces(game, current);

    int size_choice;
    int types_disponibles = 0;
    int derniere_taille_vue = 0;

    for (int i = 0; i < 3; i++)
    {
      if (current_counts[i] > 0)
      {
        types_disponibles++;
        derniere_taille_vue = i + 1;
      }
    }

    if (types_disponibles == 1)
    {
      size_choice = derniere_taille_vue;
      printf("\nSeule taille disponible : %d (Sélection automatique)\n", size_choice);
    }
    else
    {
      char prompt[128];
      char options_str[32] = "";
      if (current_counts[0] > 0)
        strcat(options_str, "1");
      if (current_counts[1] > 0)
      {
        if (options_str[0])
          strcat(options_str, "/");
        strcat(options_str, "2");
      }
      if (current_counts[2] > 0)
      {
        if (options_str[0])
          strcat(options_str, "/");
        strcat(options_str, "3");
      }

      sprintf(prompt, "\nTaille (%s) : ", options_str);

      bool valid_input = false;
      while (!valid_input)
      {
        if (!read_int_min_max(prompt, 1, 3, &size_choice))
          return false;
        if (current_counts[size_choice - 1] > 0)
          valid_input = true;
        else
          printf("Stock épuisé pour la taille %d. Choix possibles : %s.\n", size_choice, options_str);
      }
    }

    // Colonne
    int column;
    int valid_cols_count = 0;
    int last_valid_col = 0;

    for (int c = 1; c <= 6; c++)
    {
      board temp_game = copy_game(game);
      if (place_piece(temp_game, (size)size_choice, current, c) == OK)
      {
        valid_cols_count++;
        last_valid_col = c;
      }
      destroy_game(temp_game);
    }

    if (valid_cols_count == 1)
    {
      column = last_valid_col;
      printf("Seule colonne valide : %d (Sélection automatique)\n", column);
    }
    else if (valid_cols_count == 0)
    {
      printf("Aucun coup possible avec cette taille !\n");
    }
    else
    {
      if (!read_board_index("Colonne de placement (1-6) : ", &column))
        return false;
    }

    return_code status = place_piece(game, (size)size_choice, current, column);

    if (status == OK)
    {
      printf("Pièce placée avec succès.\n\n");
      current_counts[size_choice - 1]--;
      current = next_player(current);
    }
    else
    {
      printf("Impossible de placer la pièce : %s.\n", return_code_message(status));
      printf("Réessayez.\n\n");
    }
  }

  printf("Tous les placements sont terminés.\n\n");
  return true;
}

bool main_loop(board game, char names[NB_PLAYERS][NAME_MAX], bool is_pve)
{
  player current = SOUTH_P;
  printf("\n%s========================================== Phase de jeu ==========================================%s\n", GREEN, RESET);

  while (get_winner(game) == NO_PLAYER)
  {
    print_board(game, names, current);

    // SI C'EST LE TOUR DU BOT
    if (is_pve && current == NORTH_P)
    {
      bot_main_turn(game, names, current);
      if (get_winner(game) != NO_PLAYER)
        break;
      current = next_player(current);
      continue;
    }

    // SI C'EST L'HUMAIN QUI JOUE
    int idx = player_index(current);
    printf("Tour de %s (%s)\n", names[idx], player_side_name(current));

    bool turn_finished = false;
    while (!turn_finished && get_winner(game) == NO_PLAYER)
    {
      player mover = picked_piece_owner(game);
      if (mover == NO_PLAYER)
      {
        int column;
        if (!read_board_index("\nChoisissez une colonne sur la ligne la plus proche de votre but (1-6) : ", &column))
          return false;

        int line = (current == NORTH_P) ? northmost_occupied_line(game) : southmost_occupied_line(game);
        return_code status = pick_piece(game, current, line, column);
        if (status != OK)
          printf("Impossible de sélectionner la pièce : %s.\n\n", return_code_message(status));
        else
          print_board(game, names, current);
        continue;
      }

      size held = picked_piece_size(game);
      int steps_left = movement_left(game);
      printf("Pièce en main : taille %d, pas restants %d.\n", held, steps_left);

      const direction directions[] = {NORTH, SOUTH, EAST, WEST, GOAL};
      const char keys[] = {'N', 'S', 'E', 'W', 'G'};
      const char *labels[] = {"Nord", "Sud", "Est", "Ouest", "But"};
      bool can_move[5] = {false};
      for (size_t i = 0; i < 5; ++i)
        can_move[i] = is_move_possible(game, directions[i]);

      printf("Actions possibles : ");
      for (size_t i = 0; i < 5; ++i)
        if (can_move[i])
          printf("[%c]=%s ", keys[i], labels[i]);
      printf("[C]=Annuler mouvement [P]=Annuler pas");
      if (steps_left == 0)
        printf(" [X]=Échanger");
      printf("\n\n> ");

      char command = 0;
      if (scanf(" %c", &command) != 1)
        return false;
      command = (char)toupper((unsigned char)command);

      if (command == 'C')
      {
        if (cancel_movement(game) == OK)
        {
          printf("Mouvement annulé.\n");
          print_board(game, names, current);
          turn_finished = true;
        }
        break;
      }
      if (command == 'P')
      {
        if (cancel_step(game) == OK)
        {
          printf("Pas annulé.\n");
          print_board(game, names, current);
        }
        continue;
      }
      if (command == 'X')
      {
        int tl, tc;
        if (!read_board_index("Ligne cible (1-6) : ", &tl) || !read_board_index("Colonne cible (1-6) : ", &tc))
          return false;
        if (swap_piece(game, tl, tc) == OK)
        {
          turn_finished = true;
          break;
        }
        else
          printf("Échange impossible.\n");
        continue;
      }

      direction desired = GOAL;
      if (command == 'N')
        desired = NORTH;
      else if (command == 'S')
        desired = SOUTH;
      else if (command == 'E')
        desired = EAST;
      else if (command == 'W')
        desired = WEST;
      else if (command == 'G')
        desired = GOAL;
      else
      {
        printf("Commande inconnue.\n");
        continue;
      }

      if (move_piece(game, desired) != OK)
      {
        printf("Mouvement impossible.\n");
      }
      else
      {
        print_board(game, names, current);
        if (picked_piece_owner(game) == NO_PLAYER)
        {
          turn_finished = true;
          break;
        }
      }
    }

    if (get_winner(game) != NO_PLAYER)
      break;
    if (!turn_finished)
      printf("Fin du tour de %s.\n\n", names[idx]);
    current = next_player(current);
  }

  player victor = get_winner(game);
  if (victor != NO_PLAYER)
  {
    if (!is_pve)
    {
      int victor_index = player_index(victor);
      printf("\nLA PARTIE EST TERMINÉE !\n");
      printf("Le joueur %s (%s) a gagné !\n", names[victor_index], player_side_name(victor));
    }
    else if (is_pve && victor == NORTH_P)
    {
      printf("%sLe Bot a remporté la partie !%s\n", RED, RESET);
    }
    else
    {
      printf("%sFélicitations, vous avez gagné contre DORBOT 👑 !%s\n", GREEN, RESET);

      display_ascii_art("dorbot.txt");
    }
  }
  return true;
}

int main()
{
  printf("\n\n");
  display_ascii_art("title.txt");
  srand(time(NULL));

  printf("Bienvenue ! Veuillez choisir le mode de jeu.\n");
  printf("[1] PvP (Joueur vs Joueur)\n");
  printf("[2] PvE (Joueur vs DorBot)\n");
  int mode = 0;
  read_int_min_max("> ", 1, 2, &mode);
  bool is_pve = (mode == 2);

  char player_names[NB_PLAYERS][NAME_MAX] = {{0}};

  if (!ask_name(player_names[0], NAME_MAX, "Sud (Vous)"))
    return 0;

  if (is_pve)
  {
    strcpy(player_names[1], "DORBOT");
  }
  else
  {
    if (!ask_name(player_names[1], NAME_MAX, "Nord"))
      return 0;
  }

  board game = new_game();
  if (!game)
    return 1;

  if (!setup_phase(game, player_names, is_pve))
  {
    destroy_game(game);
    return 0;
  }

  if (!main_loop(game, player_names, is_pve))
  {
    destroy_game(game);
    return 0;
  }

  destroy_game(game);
  return 0;
}