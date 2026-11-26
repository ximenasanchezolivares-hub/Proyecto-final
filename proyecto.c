#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define nombre_max 50
#define desc 100
#define motivo_max 80
#define archivo_expedientes "expedientes.txt"
#define pendiente 0
#define aprobada  1
#define rechazada 2
#define revision  3

typedef struct {
    int id_solicitud;
    int id_ciudadano;
    char descripcion[desc];
    int nivel_ridiculez;
    int urgencia;
    float costo;
    int estado;
} Solicitud;

typedef struct {
    int id_ciudadano;
    char nombre[nombre_max];
    char contacto[nombre_max];
    Solicitud *tramites;
    int cant_tramites;
    int cap_tramites;
} Ciudadano;

typedef struct {
    int folio;
    int id_solicitud;
    int id_ciudadano;
    int decision;
    float puntuacion;
    char motivo[motivo_max];
} Resolucion;

typedef struct {
    Ciudadano *ciudadanos;
    int cant_ciudadanos;
    int cap_ciudadanos;

    Resolucion *historial;
    int cant_historial;
    int cap_historial;

    int sig_id_solicitud;
    int sig_folio;
} Ministerio;

Ministerio *ministerio_crear(int cap_inicial) {
    if (cap_inicial <= 0) cap_inicial = 2;

    Ministerio *m = malloc(sizeof(Ministerio));
    if (m == NULL) return NULL;

    m->ciudadanos = malloc((size_t)cap_inicial * sizeof(Ciudadano));
    if (m->ciudadanos == NULL) {
        free(m);
        return NULL;
    }

    m->historial = malloc((size_t)cap_inicial * sizeof(Resolucion));
    if (m->historial == NULL) {
        free(m->ciudadanos);
        free(m);
        return NULL;
    }

    m->cant_ciudadanos = 0;
    m->cap_ciudadanos = cap_inicial;
    m->cant_historial = 0;
    m->cap_historial = cap_inicial;
    m->sig_id_solicitud = 1;
    m->sig_folio = 100;

    return m;
}

Ciudadano *buscar_ciudadano(const Ministerio *m, int id) {
    if (m == NULL || m->ciudadanos == NULL) return NULL;
    for (int i = 0; i < m->cant_ciudadanos; i++) {
        if (m->ciudadanos[i].id_ciudadano == id) {
            return &m->ciudadanos[i];
        }
    }
    return NULL;
}

int registrar_ciudadano(Ministerio *m, int id, const char *nombre, const char *contacto) {
    if (m == NULL || m->ciudadanos == NULL) return 0;
    if (id <= 0 || nombre == NULL || contacto == NULL) return 0;
    if (nombre[0] == '\0' || contacto[0] == '\0') return 0;
    if (buscar_ciudadano(m, id) != NULL) return 0;

    if (m->cant_ciudadanos == m->cap_ciudadanos) {
        int nueva_cap = m->cap_ciudadanos * 2;
        Ciudadano *tmp = realloc(m->ciudadanos, (size_t)nueva_cap * sizeof(Ciudadano));
        if (tmp == NULL) return 0;
        m->ciudadanos = tmp;
        m->cap_ciudadanos = nueva_cap;
    }

    Ciudadano *nuevo = &m->ciudadanos[m->cant_ciudadanos];
    nuevo->id_ciudadano = id;
    strncpy(nuevo->nombre, nombre, nombre_max - 1);
    nuevo->nombre[nombre_max - 1] = '\0';
    strncpy(nuevo->contacto, contacto, nombre_max - 1);
    nuevo->contacto[nombre_max - 1] = '\0';

    nuevo->cap_tramites = 2;
    nuevo->cant_tramites = 0;
    nuevo->tramites = malloc((size_t)nuevo->cap_tramites * sizeof(Solicitud));
    if (nuevo->tramites == NULL) {
        nuevo->cap_tramites = 0;
        return 0;
    }

    m->cant_ciudadanos++;
    return 1;
}

int agregar_solicitud(Ministerio *m, int id_ciudadano, const char *descripcion, int ridiculez, int urgencia, float costo) {
    if (m == NULL || descripcion == NULL || descripcion[0] == '\0') return 0;
    if (ridiculez < 1 || ridiculez > 10) return 0;
    if (urgencia < 1 || urgencia > 10) return 0;
    if (costo < 0.0f) return 0;

    Ciudadano *c = buscar_ciudadano(m, id_ciudadano);
    if (c == NULL || c->tramites == NULL) return 0;

    if (c->cant_tramites == c->cap_tramites) {
        int nueva_cap = c->cap_tramites * 2;
        Solicitud *tmp = realloc(c->tramites, (size_t)nueva_cap * sizeof(Solicitud));
        if (tmp == NULL) return 0;
        c->tramites = tmp;
        c->cap_tramites = nueva_cap;
    }

    Solicitud *s = &c->tramites[c->cant_tramites];
    s->id_solicitud = m->sig_id_solicitud++;
    s->id_ciudadano = c->id_ciudadano;
    strncpy(s->descripcion, descripcion, desc - 1);
    s->descripcion[desc - 1] = '\0';
    s->nivel_ridiculez = ridiculez;
    s->urgencia = urgencia;
    s->costo = costo;
    s->estado = pendiente;

    c->cant_tramites++;
    return 1;
}

Solicitud *buscar_solicitud(Ministerio *m, int id_solicitud, Ciudadano **ciudadano_out) {
    if (m == NULL || m->ciudadanos == NULL || id_solicitud <= 0) return NULL;

    for (int i = 0; i < m->cant_ciudadanos; i++) {
        Ciudadano *c = &m->ciudadanos[i];
        for (int j = 0; j < c->cant_tramites; j++) {
            if (c->tramites[j].id_solicitud == id_solicitud) {
                if (ciudadano_out != NULL) {
                    *ciudadano_out = c;
                }
                return &c->tramites[j];
            }
        }
    }
    return NULL;
}

float calcular_puntuacion_absurda(const Solicitud *s) {
    if (s == NULL) return 0.0f;

    return ((float)s->nivel_ridiculez * 2.0f) +
           ((float)s->urgencia * 1.5f) -
           (s->costo * 0.05f);
}

int sugerir_clasificacion(const Solicitud *s) {
    if (s == NULL) return rechazada;

    float pts = calcular_puntuacion_absurda(s);
    if (s->nivel_ridiculez >= 8 && s->urgencia >= 6 && pts >= 20.0f) {
        return aprobada;
    }
    if (pts >= 10.0f) {
        return revision;
    }
    return rechazada;
}

int resolver_solicitud(Ministerio *m, int id_solicitud, int decision, const char *motivo) {
    if (m == NULL || m->historial == NULL) return 0;
    if (decision != aprobada && decision != rechazada && decision != revision) return 0;

    Ciudadano *dueno = NULL;
    Solicitud *s = buscar_solicitud(m, id_solicitud, &dueno);
    if (s == NULL || dueno == NULL) return 0;

    if (m->cant_historial == m->cap_historial) {
        int nueva_cap = m->cap_historial * 2;
        Resolucion *tmp = realloc(m->historial, (size_t)nueva_cap * sizeof(Resolucion));
        if (tmp == NULL) return 0;
        m->historial = tmp;
        m->cap_historial = nueva_cap;
    }

    s->estado = decision;

    Resolucion *r = &m->historial[m->cant_historial];
    r->folio = m->sig_folio++;
    r->id_solicitud = s->id_solicitud;
    r->id_ciudadano = dueno->id_ciudadano;
    r->decision = decision;
    r->puntuacion = calcular_puntuacion_absurda(s);

    if (motivo != NULL && motivo[0] != '\0') {
        strncpy(r->motivo, motivo, motivo_max - 1);
        r->motivo[motivo_max - 1] = '\0';
    } else {
        snprintf(r->motivo, motivo_max, "resolucion del folio %d", r->folio);
    }

    m->cant_historial++;
    return 1;
}

static const char *nombre_estado(int estado) {
    switch (estado) {
        case pendiente: return "pendiente";
        case aprobada:  return "aprobada";
        case rechazada: return "rechazada";
        case revision:  return "en revision";
        default:        return "desconocido";
    }
}

void mostrar_historial_y_estadisticas(const Ministerio *m) {
    if (m == NULL) return;

    if (m->cant_historial == 0) {
        printf("no hay resoluciones registradas\n");
    } else {
        for (int i = 0; i < m->cant_historial; i++) {
            const Resolucion *r = &m->historial[i];
            printf("folio %d | solicitud %d | ciudadano %d | estado: %s | pts: %.2f | motivo: %s\n",
                   r->folio, r->id_solicitud, r->id_ciudadano,
                   nombre_estado(r->decision), r->puntuacion, r->motivo);
        }
    }

    int total_tramites = 0;
    int c_pend = 0, c_aprob = 0, c_rech = 0, c_rev = 0;
    long suma_rid = 0, suma_urg = 0;
    double suma_costo = 0.0;

    for (int i = 0; i < m->cant_ciudadanos; i++) {
        const Ciudadano *c = &m->ciudadanos[i];
        for (int j = 0; j < c->cant_tramites; j++) {
            const Solicitud *s = &c->tramites[j];
            total_tramites++;
            suma_rid += s->nivel_ridiculez;
            suma_urg += s->urgencia;
            suma_costo += (double)s->costo;

            if (s->estado == pendiente) c_pend++;
            else if (s->estado == aprobada) c_aprob++;
            else if (s->estado == rechazada) c_rech++;
            else if (s->estado == revision) c_rev++;
        }
    }

    printf("estadisticas\n");
    printf("ciudadanos registrados: %d\n", m->cant_ciudadanos);
    printf("total de solicitudes: %d\n", total_tramites);
    printf("pendientes: %d | aprobadas: %d | rechazadas: %d | en revision: %d\n",
           c_pend, c_aprob, c_rech, c_rev);

    if (total_tramites > 0) {
        printf("promedio ridiculez: %.2f | promedio urgencia: %.2f | costo promedio: %.2f\n",
               (double)suma_rid / total_tramites,
               (double)suma_urg / total_tramites,
               suma_costo / total_tramites);
    } else {
        printf("sin solicitudes\n");
    }
}

int guardar_expedientes(const Ministerio *m, const char *ruta) {
    if (m == NULL || ruta == NULL) return 0;

    FILE *f = fopen(ruta, "w");
    if (f == NULL) {
        printf("error al abrir el archivo '%s' para guardar.\n", ruta);
        return 0;
    }

    fprintf(f, "ciudadanos %d\n", m->cant_ciudadanos);
    for (int i = 0; i < m->cant_ciudadanos; i++) {
        const Ciudadano *c = &m->ciudadanos[i];
        fprintf(f, "%d|%s|%s|%d\n", c->id_ciudadano, c->nombre, c->contacto, c->cant_tramites);
        for (int j = 0; j < c->cant_tramites; j++) {
            const Solicitud *s = &c->tramites[j];
            fprintf(f, "SOL|%d|%d|%d|%d|%.2f|%d|%s\n",
                    s->id_solicitud, s->id_ciudadano, s->nivel_ridiculez,
                    s->urgencia, s->costo, s->estado, s->descripcion);
        }
    }

    fprintf(f, "historial %d\n", m->cant_historial);
    for (int i = 0; i < m->cant_historial; i++) {
        const Resolucion *r = &m->historial[i];
        fprintf(f, "RES|%d|%d|%d|%d|%.2f|%s\n",
                r->folio, r->id_solicitud, r->id_ciudadano,
                r->decision, r->puntuacion, r->motivo);
    }

    fclose(f);
    return 1;
}

int cargar_expedientes(Ministerio *m, const char *ruta) {
    if (m == NULL || ruta == NULL || ruta[0] == '\0') return 0;

    FILE *f = fopen(ruta, "r");
    if (f == NULL) {
        return 0;
    }

    int total_ciud = 0;
    if (fscanf(f, "ciudadanos %d\n", &total_ciud) != 1) {
        fclose(f);
        return 0;
    }

    int max_id_sol = 0;
    for (int i = 0; i < total_ciud; i++) {
        int id_c = 0, n_tram = 0;
        char nom[nombre_max] = "", cont[nombre_max] = "";
        if (fscanf(f, "%d|%49[^|]|%49[^|]|%d\n", &id_c, nom, cont, &n_tram) != 4) {
            break;
        }

        if (registrar_ciudadano(m, id_c, nom, cont) && n_tram > 0) {
            for (int j = 0; j < n_tram; j++) {
                int id_s = 0, id_dueno = 0, rid = 1, urg = 1, est = pendiente;
                float costo = 0.0f;
                char d_str[desc] = "";
                if (fscanf(f, "SOL|%d|%d|%d|%d|%f|%d|%99[^\n]\n",
                           &id_s, &id_dueno, &rid, &urg, &costo, &est, d_str) == 7) {
                    if (agregar_solicitud(m, id_c, d_str, rid, urg, costo)) {
                        Ciudadano *c_ptr = buscar_ciudadano(m, id_c);
                        if (c_ptr != NULL && c_ptr->cant_tramites > 0) {
                            c_ptr->tramites[c_ptr->cant_tramites - 1].id_solicitud = id_s;
                            c_ptr->tramites[c_ptr->cant_tramites - 1].estado = est;
                        }
                        if (id_s > max_id_sol) max_id_sol = id_s;
                    }
                }
            }
        }
    }
    if (max_id_sol >= m->sig_id_solicitud) {
        m->sig_id_solicitud = max_id_sol + 1;
    }

    int total_hist = 0, max_folio = m->sig_folio - 1;
    if (fscanf(f, "historial %d\n", &total_hist) == 1) {
        for (int i = 0; i < total_hist; i++) {
            int fol = 0, id_s = 0, id_c = 0, dec = revision;
            float pts = 0.0f;
            char mot[motivo_max] = "";
            if (fscanf(f, "RES|%d|%d|%d|%d|%f|%79[^\n]\n",
                       &fol, &id_s, &id_c, &dec, &pts, mot) == 6) {
                if (m->cant_historial == m->cap_historial) {
                    int n_cap = m->cap_historial * 2;
                    Resolucion *tmp = realloc(m->historial, (size_t)n_cap * sizeof(Resolucion));
                    if (tmp != NULL) {
                        m->historial = tmp;
                        m->cap_historial = n_cap;
                    }
                }
                if (m->cant_historial < m->cap_historial) {
                    Resolucion *r = &m->historial[m->cant_historial++];
                    r->folio = fol;
                    r->id_solicitud = id_s;
                    r->id_ciudadano = id_c;
                    r->decision = dec;
                    r->puntuacion = pts;
                    strncpy(r->motivo, mot, motivo_max - 1);
                    r->motivo[motivo_max - 1] = '\0';
                    if (fol > max_folio) max_folio = fol;
                }
            }
        }
    }
    if (max_folio >= m->sig_folio) {
        m->sig_folio = max_folio + 1;
    }

    fclose(f);
    return 1;
}

int modificar_solicitud(Ministerio *m, int id_solicitud, const char *nueva_desc,
                        int ridiculez, int urgencia, float costo) {
    if (m == NULL || nueva_desc == NULL || nueva_desc[0] == '\0') return 0;
    if (ridiculez < 1 || ridiculez > 10) return 0;
    if (urgencia < 1 || urgencia > 10) return 0;
    if (costo < 0.0f) return 0;

    Solicitud *s = buscar_solicitud(m, id_solicitud, NULL);
    if (s == NULL) return 0;

    strncpy(s->descripcion, nueva_desc, desc - 1);
    s->descripcion[desc - 1] = '\0';
    s->nivel_ridiculez = ridiculez;
    s->urgencia = urgencia;
    s->costo = costo;
    return 1;
}

int eliminar_solicitud(Ministerio *m, int id_solicitud) {
    if (m == NULL || m->ciudadanos == NULL || id_solicitud <= 0) return 0;

    for (int i = 0; i < m->cant_ciudadanos; i++) {
        Ciudadano *c = &m->ciudadanos[i];
        for (int j = 0; j < c->cant_tramites; j++) {
            if (c->tramites[j].id_solicitud == id_solicitud) {
                for (int k = j; k < c->cant_tramites - 1; k++) {
                    c->tramites[k] = c->tramites[k + 1];
                }
                c->cant_tramites--;
                return 1;
            }
        }
    }
    return 0;
}

void probar_consistencia(Ministerio *m) {
    printf("ID invalido (-5): %s\n",
           registrar_ciudadano(m, -5, "invalido", "oswaldo@gmail.com") ? "error" : "rechazado");
    printf("ID duplicado (1): %s\n",
           registrar_ciudadano(m, 1, "duplicado", "liz@gmail.com") ? "error" : "rechazado");
    printf("ridiculez fuera de rango (99): %s\n",
           agregar_solicitud(m, 1, "prueba", 99, 5, 10.0f) ? "error" : "rechazado");
    printf("solicitud inexistente (9999): %s\n",
           resolver_solicitud(m, 9999, aprobada, "no existe") ? "error" : "rechazado");
    printf("apuntador NULL: %s\n",
           calcular_puntuacion_absurda(NULL) == 0.0f ? "manejado" : "error");
}

void ministerio_destruir(Ministerio *m) {
    if (m == NULL) return;

    for (int i = 0; i < m->cant_ciudadanos; i++) {
        free(m->ciudadanos[i].tramites);
    }

    free(m->ciudadanos);
    free(m->historial);
    free(m);
}

int main(void) {
    Ministerio *min = ministerio_crear(2);
    if (min == NULL) {
        printf("no se pudo reservar en la memoria.\n");
        return 1;
    }

    cargar_expedientes(min, archivo_expedientes);

    if (registrar_ciudadano(min, 1, "ximena sanches olivares", "ximena@gmail.com")) {
        agregar_solicitud(min, 1, "permiso para bostezar el martes", 8, 6, 120.0f);
        agregar_solicitud(min, 1, "licencia para sonar con pinguinos", 10, 9, 50.0f);
        printf("ciudadano registrado. tramite total de ximena: %d\n", min->ciudadanos[0].cant_tramites);
    }

    if (min->cant_ciudadanos > 0 && min->ciudadanos[0].cant_tramites > 0) {
        Solicitud *primera = &min->ciudadanos[0].tramites[0];
        float pts = calcular_puntuacion_absurda(primera);
        int sugerida = sugerir_clasificacion(primera);

        printf("tramite %d puntuacion: %.2f | clasificacion sugerida: %d\n",
               primera->id_solicitud, pts, sugerida);

        if (resolver_solicitud(min, primera->id_solicitud, sugerida, "evaluado por algoritmo del ministerio")) {
            printf("resolucion registrada : %d\n", min->cant_historial);
        }
    }

    probar_consistencia(min);
    mostrar_historial_y_estadisticas(min);

    if (guardar_expedientes(min, archivo_expedientes)) {
        printf("expediente guardado '%s'\n", archivo_expedientes);
    }

    ministerio_destruir(min);
    return 0;
}